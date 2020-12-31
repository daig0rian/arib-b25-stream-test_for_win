#define VERSION_STRING "0.2.7"

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <io.h>
#include <windows.h>
#include <crtdbg.h>
#include <tchar.h>
#else
#define __STDC_FORMAT_MACROS
#define TCHAR char
#define _T(X) X
#define _ftprintf fprintf
#define _ttoi atoi
#define _tmain main
#define _topen _open
#include <unistd.h>
#include <sys/time.h>
#endif

#include "arib_std_b25.h"
#include "arib_std_b25_error_code.h"
#include "b_cas_card.h"

typedef struct {
	int32_t round;
	int32_t strip;
	int32_t emm;
	int32_t verbose;
	int32_t power_ctrl;
} OPTION;

static void show_usage();
static int parse_arg(OPTION *dst, int argc, TCHAR **argv);
static void test_arib_std_b25(OPTION *opt);
static void show_bcas_power_on_control_info(B_CAS_CARD *bcas);

int _tmain(int argc, TCHAR **argv)
{
	int n;
	OPTION opt;

#if defined(_WIN32)
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	n = parse_arg(&opt, argc, argv);
	if (n != argc) {
		show_usage();
		exit(EXIT_FAILURE);
	}


	test_arib_std_b25(&opt);


#if defined(_WIN32)
	_CrtDumpMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}

static void show_usage()
{
	_ftprintf(stderr, _T("arib-b25-stream-test.exe - base on b25.exe version %s \n"), _T(VERSION_STRING));
	_ftprintf(stderr, _T("usage: arib-b25-stream-test.exe [options] \n"));
	_ftprintf(stderr, _T("options:\n"));
	_ftprintf(stderr, _T("  -r round (integer, default=4)\n"));
	_ftprintf(stderr, _T("  -s strip\n"));
	_ftprintf(stderr, _T("     0: keep null(padding) stream\n"));
	_ftprintf(stderr, _T("     1: strip null stream (default)\n"));
	_ftprintf(stderr, _T("  -m EMM\n"));
	_ftprintf(stderr, _T("     0: ignore EMM (default)\n"));
	_ftprintf(stderr, _T("     1: send EMM to B-CAS card\n"));
	_ftprintf(stderr, _T("  -p power_on_control_info\n"));
	_ftprintf(stderr, _T("     0: do nothing additionaly\n"));
	_ftprintf(stderr, _T("     1: show B-CAS EMM receiving request (default)\n"));
	_ftprintf(stderr, _T("  -v verbose\n"));
	_ftprintf(stderr, _T("     0: silent (default)\n"));
	_ftprintf(stderr, _T("     1: show processing status\n"));
	_ftprintf(stderr, _T("\n"));
}

static int parse_arg(OPTION *dst, int argc, TCHAR **argv)
{
	int i;

	dst->round = 4;
	dst->strip = 1;
	dst->emm = 0;
	dst->power_ctrl = 1;
	dst->verbose = 0;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			break;
		}
		switch (argv[i][1]) {
		case 'm':
			if (argv[i][2]) {
				dst->emm = _ttoi(argv[i] + 2);
			}
			else {
				dst->emm = _ttoi(argv[i + 1]);
				i += 1;
			}
			break;
		case 'p':
			if (argv[i][2]) {
				dst->power_ctrl = _ttoi(argv[i] + 2);
			}
			else {
				dst->power_ctrl = _ttoi(argv[i + 1]);
				i += 1;
			}
			break;
		case 'r':
			if (argv[i][2]) {
				dst->round = _ttoi(argv[i] + 2);
			}
			else {
				dst->round = _ttoi(argv[i + 1]);
				i += 1;
			}
			break;
		case 's':
			if (argv[i][2]) {
				dst->strip = _ttoi(argv[i] + 2);
			}
			else {
				dst->strip = _ttoi(argv[i + 1]);
				i += 1;
			}
			break;
		case 'v':
			if (argv[i][2]) {
				dst->verbose = _ttoi(argv[i] + 2);
			}
			else {
				dst->verbose = _ttoi(argv[i + 1]);
				i += 1;
			}
			break;
		default:
			_ftprintf(stderr, _T("error - unknown option '-%c'\n"), argv[i][1]);
			return argc;
		}
	}

	return i;
}

static void test_arib_std_b25(OPTION *opt)
{
	int code, i, n;
	int sfd, dfd;


	int32_t offset;
#if defined(_WIN32)
	unsigned long tick, tock;
#else
	struct timeval tick, tock;
	double millisec;
#endif
	double mbps;

	ARIB_STD_B25 *b25;
	B_CAS_CARD   *bcas;

	ARIB_STD_B25_PROGRAM_INFO pgrm;

	uint8_t data[64 * 1024];
	uint8_t *_data;

	ARIB_STD_B25_BUFFER sbuf;
	ARIB_STD_B25_BUFFER dbuf;

	sfd = 0; //stdin
	dfd = 1; //stdout
	b25 = NULL;
	bcas = NULL;
	_data = NULL;

	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	b25 = create_arib_std_b25();
	if (b25 == NULL) {
		_ftprintf(stderr, _T("error - failed on create_arib_std_b25()\n"));
		goto LAST;
	}

	code = b25->set_multi2_round(b25, opt->round);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::set_multi2_round() : code=%d\n"), code);
		goto LAST;
	}

	code = b25->set_strip(b25, opt->strip);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::set_strip() : code=%d\n"), code);
		goto LAST;
	}

	code = b25->set_emm_proc(b25, opt->emm);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::set_emm_proc() : code=%d\n"), code);
		goto LAST;
	}

	bcas = create_b_cas_card();
	if (bcas == NULL) {
		_ftprintf(stderr, _T("error - failed on create_b_cas_card()\n"));
		goto LAST;
	}

	code = bcas->init(bcas);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on B_CAS_CARD::init() : code=%d\n"), code);
		goto LAST;
	}

	code = b25->set_b_cas_card(b25, bcas);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::set_b_cas_card() : code=%d\n"), code);
		goto LAST;
	}

	offset = 0;
#if defined(_WIN32)
	tock = GetTickCount();
#else
	gettimeofday(&tock, NULL);
#endif
	while ((n = _read(sfd, data, sizeof(data))) > 0) {
		sbuf.data = data;
		sbuf.size = n;

		code = b25->put(b25, &sbuf);
		if (code < 0) {
			_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::put() : code=%d\n"), code);
			dbuf.data = data;
			dbuf.size = n;
			if (code < ARIB_STD_B25_ERROR_NO_ECM_IN_HEAD_32M) {
				uint8_t *p = NULL;
				b25->withdraw(b25, &sbuf);
				if (sbuf.size > 0) {
					if (_data != NULL) {
						free(_data);
						_data = NULL;
					}
					p = (uint8_t *)malloc(sbuf.size + n);
				}
				if (p != NULL) {
					memcpy(p, sbuf.data, sbuf.size);
					memcpy(p + sbuf.size, data, n);
					dbuf.data = p;
					dbuf.size = sbuf.size + n;
					_data = p;
				}
			}
		}
		else {
			code = b25->get(b25, &dbuf);
			if (code < 0) {
				_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::get() : code=%d\n"), code);
				goto LAST;
			}
		}

		if (dbuf.size > 0) {
			n = _write(dfd, dbuf.data, dbuf.size);
			if (n != dbuf.size) {
				_ftprintf(stderr, _T("error failed on _write(%d)\n"), dbuf.size);
				goto LAST;
			}
		}

		offset += sbuf.size;
		if (opt->verbose != 0) {
			mbps = 0.0;
#if defined(_WIN32)
			tick = GetTickCount();
			if (tick - tock > 100) {
				mbps = offset;
				mbps /= 1024;
				mbps /= (tick - tock);
			}
#else
			gettimeofday(&tick, NULL);
			millisec = (tick.tv_sec - tock.tv_sec) * 1000;
			millisec += (tick.tv_usec - tock.tv_usec) / 1000;
			if (millisec > 100.0) {
				mbps = offset;
				mbps /= 1024;
				mbps /= millisec;
			}
#endif
			_ftprintf(stderr, _T("\rprocessing: %6.2f MB/sec"), mbps);
		}
	}

	code = b25->flush(b25);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::flush() : code=%d\n"), code);
		goto LAST;
	}

	code = b25->get(b25, &dbuf);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::get() : code=%d\n"), code);
		goto LAST;
	}

	if (dbuf.size > 0) {
		n = _write(dfd, dbuf.data, dbuf.size);
		if (n != dbuf.size) {
			_ftprintf(stderr, _T("error - failed on _write(%d)\n"), dbuf.size);
			goto LAST;
		}
	}

	if (opt->verbose != 0) {
		mbps = 0.0;
#if defined(_WIN32)
		tick = GetTickCount();
		if (tick - tock > 100) {
			mbps = offset;
			mbps /= 1024;
			mbps /= (tick - tock);
		}
#else
		gettimeofday(&tick, NULL);
		millisec = (tick.tv_sec - tock.tv_sec) * 1000;
		millisec += (tick.tv_usec - tock.tv_usec) / 1000;
		if (millisec > 100.0) {
			mbps = offset;
			mbps /= 1024;
			mbps /= millisec;
		}
#endif
		_ftprintf(stderr, _T("\rprocessing: finish  [%6.2f MB/sec]\n"), mbps);
		fflush(stderr);
		fflush(stdout);
	}

	n = b25->get_program_count(b25);
	if (n < 0) {
		_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::get_program_count() : code=%d\n"), code);
		goto LAST;
	}
	for (i = 0; i < n; i++) {
		code = b25->get_program_info(b25, &pgrm, i);
		if (code < 0) {
			_ftprintf(stderr, _T("error - failed on ARIB_STD_B25::get_program_info(%d) : code=%d\n"), i, code);
			goto LAST;
		}
		if (pgrm.ecm_unpurchased_count > 0) {
			_ftprintf(stderr, _T("warning - unpurchased ECM is detected\n"));
			_ftprintf(stderr, _T("  channel:               %d\n"), pgrm.program_number);
			_ftprintf(stderr, _T("  unpurchased ECM count: %d\n"), pgrm.ecm_unpurchased_count);
			_ftprintf(stderr, _T("  last ECM error code:   %04x\n"), pgrm.last_ecm_error_code);
			_ftprintf(stderr, _T("  undecrypted TS packet: %" PRId64 "\n"), pgrm.undecrypted_packet_count);
			_ftprintf(stderr, _T("  total TS packet:       %" PRId64 "\n"), pgrm.total_packet_count);
		}
	}

	if (opt->power_ctrl != 0) {
		show_bcas_power_on_control_info(bcas);
	}

LAST:

	if (_data != NULL) {
		free(_data);
		_data = NULL;
	}


	if (b25 != NULL) {
		b25->release(b25);
		b25 = NULL;
	}

	if (bcas != NULL) {
		bcas->release(bcas);
		bcas = NULL;
	}
}

static void show_bcas_power_on_control_info(B_CAS_CARD *bcas)
{
	int code;
	int i, w;
	B_CAS_PWR_ON_CTRL_INFO pwc;

	code = bcas->get_pwr_on_ctrl(bcas, &pwc);
	if (code < 0) {
		_ftprintf(stderr, _T("error - failed on B_CAS_CARD::get_pwr_on_ctrl() : code=%d\n"), code);
		return;
	}

	if (pwc.count == 0) {
		_ftprintf(stderr, _T("no EMM receiving request\n"));
		return;
	}

	_ftprintf(stderr, _T("total %d EMM receiving request\n"), pwc.count);
	for (i = 0; i < pwc.count; i++) {
		_ftprintf(stderr, _T("+ [%d] : tune "), i);
		switch (pwc.data[i].network_id) {
		case 4:
			w = pwc.data[i].transport_id;
			_ftprintf(stderr, _T("BS-%d/TS-%d "), ((w >> 4) & 0x1f), (w & 7));
			break;
		case 6:
		case 7:
			w = pwc.data[i].transport_id;
			_ftprintf(stderr, _T("ND-%d/TS-%d "), ((w >> 4) & 0x1f), (w & 7));
			break;
		default:
			_ftprintf(stderr, _T("unknown(b:0x%02x,n:0x%04x,t:0x%04x) "), pwc.data[i].broadcaster_group_id, pwc.data[i].network_id, pwc.data[i].transport_id);
			break;
		}
		_ftprintf(stderr, _T("between %04d %02d/%02d "), pwc.data[i].s_yy, pwc.data[i].s_mm, pwc.data[i].s_dd);
		_ftprintf(stderr, _T("to %04d %02d/%02d "), pwc.data[i].l_yy, pwc.data[i].l_mm, pwc.data[i].l_dd);
		_ftprintf(stderr, _T("least %d hours\n"), pwc.data[i].hold_time);
	}
}
