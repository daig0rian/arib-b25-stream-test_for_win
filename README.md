# arib-b25-stream-test.exe
This is test program, referencing [arib-b25-stream-test](https://www.npmjs.com/package/arib-b25-stream-test). 
This is forked from [epgdatacapbon/libaribb25](https://github.com/epgdatacapbon/libaribb25)

# How to test 1
As a decoder for mirakurun on Windows.
```    
c:\> notepad.exe %USERPROFILE%\.Mirakurun\tuners.yml
```
```
-------------
- name: PX_x3U4_S1
  types:
    - BS
    - CS
  command: BonRecTest.exe --driver BonDriver_PX_x3U4_S.dll --output - --space <space> --channel <channel>
  decoder: arib-b25-stream-test.exe
  isDisabled: false
-------------
```
# How to test 2
```
c:\> BonRecTest.exe --log --driver BonDriver_PX_x3U4_T.dll --output - --channel 14 | arib-b25-stream-test.exe | ffplay.exe -i -
```
# How to test 3
```
c:\> curl --silent http://192.168.0.3:40772/api/services/3273601024/stream?decode=0 | arib-b25-stream-test.exe -v 1 > rectest.ts
```
# How to build
```
C:\my\work\path> git clone https://github.com/daig0rian/arib-b25-stream-test_for_win
C:\my\work\path> cd arib-b25-stream-test_for_win
C:\my\work\path\arib-b25-stream-test_for_win> msbuild /p:Configuration=Release /p:Platform="Win32"
```    
... and you have got a `arib-b25-stream-test.exe` on `C:\my\work\path\arib-b25-stream-test_for_win\Win32\Release` .



# How to install build tools.
Install `git` and `VC++ BuildTools` with `Chocolatey`.
```
PowerShell(Run as administrator)
PS C:\WINDOWS\system32> # Chocolatey
PS C:\WINDOWS\system32> # https://chocolatey.org/install#installing-chocolatey
PS C:\WINDOWS\system32> Set-ExecutionPolicy Bypass -Scope Process -Force; `
                        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
                        iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
PS C:\WINDOWS\system32> choco install git.install 
PS C:\WINDOWS\system32> choco install visualstudio2017buildtools
```
Install `Windows 10 SDK (10.0.16299.0)` with `Visual Studio Installer` that on your Windows-Start-Menu.


# Original document
【名称】

　ARIB STD-B25 仕様確認テストプログラム

【バージョン】

　0.2.7

【オリジナル作者】

　茂木 和洋 (MOGI, Kazuhiro) 
　kazhiro@marumo.ne.jp

【一次配布元】

　http://www.marumo.ne.jp/db2012_2.htm#13 又は

　あるいは

　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.5.lzh

【目的】

　ARIB STD-B25 の仕様を理解する為の、参考用の実装として公開

【背景】

　2011 年 7 月の地上アナログ放送停波を控え、廉価な地上デジタル放送
　受信機の販売が待たれている

　しかし、ARIB の標準文書はわざと判りにくく書いて開発費をかさませ
　ようとしているとしか思えないほどに意味不明瞭な記述になっており
　このままでは低価格受信機の開発など不可能に思える

　そこで、自分なりに ARIB 標準文書を読み、理解した範囲をソース
　コードの形にまとめて公開することにした

　このコードが安価な受信機の開発の一助となることを期待する

　なお、あくまでも仕様理解を目的としたものであるため、ビルド済み
　バイナリファイルは配布しない

【実装した範囲】

　CA システム (B-CAS カード関連) を中心に ECM(table_id=0x82) の処理と
　ストリーム暗号の復号処理、EMM(table_id=0x84) の処理までを実装した

　EMM メッセージ (table_id=0x85) 関連は未実装となっている

【プログラムの動作環境】

　ISO 7816 対応の IC カードリーダがインストールされた Windows PC を
　想定動作環境とする

　ISO 7816 対応スマートカードリーダーは一般に
　「住基カード対応 IC カードリーダ」「e-Tax 対応 IC カードリーダ」
　などとして 4000 円程度で販売されているものが利用可能である

　日立マクセル製の HX-520UJJ と NTT コミュニケーションズの SCR3310 
　で正常に動作することを確認している

【ソースコードのライセンスについて】

　・ソースコードを利用したことによって、特許上のトラブルが発生しても
　　茂木 和洋は責任を負わない
　・ソースコードを利用したことによって、プログラムに問題が発生しても
　　茂木 和洋は責任を負わない

　上記 2 条件に同意して作成された二次的著作物に対して、茂木 和洋は
　原著作者に与えられる諸権利を行使しない

【バイナリの構成】

　・b25.exe / b25
　　ARIB STD-B25 記載の処理を行うためのプログラム

　・libaribb25.dll / libaribb25.so
　　MULTI2 復号処理を行うライブラリ
　　libaribb25.dll は B25Decoder.dll と互換性がある

【プログラムの構成】

　・arib_std_b25.h/c

　　ARIB STD-B25 記載の処理を行うためのモジュール
　　MPEG-2 TS の分離、CA システム (B-CAS カード) 機能の呼び出し、
　　MULTI2 復号機能の呼び出し等を担当する

　・ts_section_parser.h/c

　　MPEG-2 TS のセクション形式データの分割処理を担当する

　・b_cas_card.h/c

　　CA システム (B-CAS カード) のリソース管理および直接の制御を
　　担当する

　・multi2.h/c

　　MULTI2 暗号の符号化と復号を担当する

　・td.c

　　テストドライバ
　　PAT/PMT/ECM を含む MPEG-2 TS ファイルを読み込み、復号後の
　　MPEG-2 TS ファイルを出力する

　　コマンドラインオプションで MULTI2 暗号のラウンド数を指定可能
　　ラウンド数を指定しない場合の初期値は 4

　　このラウンド数 4 は MULTI2 用語では 32 に相当する

　　ARIB STD-B25 では MULTI2 のラウンド数は非公開パラメータだが
　　総当たりで実際のラウンド数は推定可能である

【コンパイルの手順（debian）】

　　$ sudo apt-get install pkg-config libpcsclite-dev
    $ make
    $ make install

【処理の流れ】

　・起動時

　　1 アプリケーションは B_CAS_CARD モジュールのインスタンスを
　　　作成し、B_CAS_CARD モジュールに、初期化を依頼する

　　1.a B_CAS_CARD モジュールは WIN32 API のスマートカード関連
　　　　API を呼び出し、CA システムに接続する
　　1.b B_CAS_CARD モジュールは ARIB STD-B25 記載の「初期条件
　　　　設定コマンドを CA システムに発行し、システム鍵 (64 byte)
　　　　初期 CBC 状態 (8 byte) を受け取る 

　　2 アプリケーションは ARIB_STD_B25 モジュールのインスタンスを
　　　作成し、B_CAS_CARD モジュールを ARIB_STD_B25 モジュールに
　　　登録する

　・データ処理時

　　1 アプリケーションは ARIB_STD_B25 モジュールに順次データを
　　　提供し、ARIB_STD_B25 モジュールから処理完了データを受け
　　　取ってファイルに出力していく

　　・ARIB_STD_B25 モジュール内

　　　1 TS パケットのユニットサイズ (188/192/204 などが一般的) が
　　　　特定されていない場合 8K まで入力データをバッファしてから、
　　　　ユニットサイズを特定する
　　　　ユニットサイズが特定できなかった場合は、エラー終了する

　　　2 PAT が発見されていない場合、PAT が発見できるまで入力
　　　　データをバッファし続ける
　　　　PAT が発見できずにバッファサイズが 16M を超過した場合
　　　　エラー終了する
　　　　PAT が発見できた場合、プログラム配列を作成し PID マップ
　　　　配列に登録する

　　　3 PAT に登録されていた PMT すべてが発見されるか、どれか
　　　　ひとつの PMT で 2 個目のセクションが到着するまで入力
　　　　データをバッファし続ける
　　　　上記条件を満たさずにバッファサイズが 32M を超過した場合
　　　　エラー終了する
　　　　PMT が到着する毎に ECM の有無を確認し、ECM が存在する
　　　　場合はデクリプタを作成してプログラムに所属するストリーム
　　　　と PID マップ上で関連付ける

　　　4 PMT に登録されていた ECM すべてが発見されるか、どれか
　　　　ひとつの ECM で 2 個目のセクションが到着するまで入力
　　　　データをバッファし続ける
　　　　上記条件を満たさずにバッファサイズが 32M を超過した場合
　　　　エラー終了する
　　　　各 ECM に対して、最初のセクションデータが到着した時点で
　　　　MULTI2 モジュールのインスタンスをデクリプタ上に作成する
　　　　ECM セクションデータは B_CAS_CARD モジュールに提供して
　　　　スクランブル鍵を受け取り、MULTI2 モジュールにシステム鍵、
　　　　初期 CBC 状態、スクランブル鍵を渡し、MULTI2 復号の準備を
　　　　行う

　　　5.a 暗号化されている TS パケットであれば、PID から対応
　　　　　ECM ストリームを特定し、デクリプタの MULTI2 モジュー
　　　　　ルに復号させて出力バッファに積む
　　　　
　　　5.b 暗号化されていない TS パケットであれば、そのまま出力
　　　　　バッファに積む

　　　5.c CAT を検出した場合、EMM の PID を取得して EMM の処理
　　　　　準備を行う

　　　5.d EMM を受け取った場合、B-CAS カード ID と比較し、自分
　　　　　宛ての EMM であれば B-CAS カードに引き渡して処理させる
　　　　　# EMM 処理オプションが指定されている場合

　　　6 ECM が更新された場合、B_CAS_CARD モジュールに処理を
　　　　依頼し、出力されたスクランブル鍵を MULTI2 モジュールに
　　　　登録する

　　　7 PMT が更新された場合、ECM PID が変化していれば新たに
　　　　デクリプタを作成して 4 に戻る

　　　8 PAT が更新された場合、プログラム配列を破棄して
　　　　3 に戻る

　・終了時

　　1 各モジュールが確保したリソースを解放する

【更新履歴】

　　最新の更新内容については github を参照

　・2012, 7/23 - ver. 0.2.6

　　github に移行

　・2012, 2/13 - ver. 0.2.5

　　WOWOW でノンスクランブル <-> スクランブル切り替え後に復号が
　　行われないことがあるバグを修正

　　http://www.marumo.ne.jp/db2012_2.htm#13 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.5.lzh

　・2009, 4/19 - ver. 0.2.4

　　終端パケットが野良パケット (PMT に記載されていない PID の
　　パケット) だった場合に、ECM が 1 つだけでも復号が行われない
　　バグを修正

　　transport_error_indicator が立っている場合はパケット処理を
　　行わず、そのまま素通しするように変更

　　http://www.marumo.ne.jp/db2009_4.htm#19 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.4.lzh

　・2008, 12/30 - ver. 0.2.3

　　CA_descriptor の解釈を行う際に CA_system_id が B-CAS カード
　　から取得したものと一致するか確認を行うように変更

　　http://www.marumo.ne.jp/db2008_c.htm#30 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.3.lzh

　・2008, 11/10 - ver. 0.2.2

　　修正ユリウス日から年月日への変換処理をより正確なものへ変更

　　TS パケットサイズの特定方法を変更

　　http://www.marumo.ne.jp/db2008_b.htm#10 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.2.lzh

　・2008, 4/9 - ver. 0.2.1

　　PAT 更新時に復号漏れが発生していたバグを修正
　　(ver. 0.2.0 でのエンバグ)

　　野良 PID (PMT に記載されていないストリーム) が存在した場合
　　TS 内の ECM がひとつだけならば、その ECM で復号する形に変更

　　EMM の B-CAS カードへの送信をオプションで選択可能に変更 (-m)
　　進捗状況の表示をオプションで選択可能に変更 (-v)
　　通電制御情報 (EMM受信用) を表示するオプションを追加 (-p)

　　http://www.marumo.ne.jp/db2008_4.htm#9 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.1.lzh

　・2008, 4/6 - ver. 0.2.0

　　EMM 対応
　　利用中の B-CAS カード ID 向けの EMM を検出した場合、EMM を
　　B-CAS カードに渡す処理を追加

　　ECM 処理の際に未契約応答が返された場合、処理負荷軽減の為、
　　以降、その PID の ECM を B-CAS カードで処理しないように変
　　更 (EMM を処理した場合は再び ECM を処理するように戻す)

　　進捗を nn.nn% の書式で標準エラー出力に表示するように変更
　　
　　http://www.marumo.ne.jp/db2008_4.htm#6 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.2.0.lzh

　・2008, 3/31 - ver. 0.1.9

　　MULTI2 モジュールのインスタンスが未作製の状況で、MULTI2 の
　　機能を呼び出して例外を発生させることがあったバグを修正

　　# パッチを提供してくれた方に感謝

　　http://www.marumo.ne.jp/db2008_3.htm#31 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.9.lzh

　・2008, 3/24 - ver. 0.1.8

　　-s オプション (NULL パケットの削除) を追加
　　-s 1 で NULL パケットを出力ファイルには保存しなくなる
　　デフォルトは -s 0 の NULL パケット保持

　　http://www.marumo.ne.jp/db2008_3.htm#24 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.8.lzh

　・2008, 3/17 - ver. 0.1.7

　　arib_std_b25.h に「extern "C" {」を閉じるコードがなかった問題 
　　(C++ コードから利用する場合にコンパイルエラーを発生させる) を
　　修正

　　TS パケットの中途でストリームが切り替わるケースで問題が発生し
　　にくくなるように、arib_std_b25.c 内のコードを修正

　　http://www.marumo.ne.jp/db2008_3.htm#17 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.7.lzh

　・2008, 3/16 - ver. 0.1.6

　　PMT 更新の際、ECM 関連の状況が変更 (スクランブル - ノンスク
　　ランブルの切り替えや、ECM PID の変更等) が行われても、それが
　　反映されていなかった問題を修正

　　http://www.marumo.ne.jp/db2008_3.htm#16 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.6.lzh

　・2008, 2/14

　　readme.txt (このファイル) を修正
　　ソースコードのライセンスについての記述を追加

　・2008, 2/12 - ver. 0.1.5

　　PMT の更新に伴い、どのプログラムにも所属しなくなった PID (スト
　　リーム) でパケットが送信され続けた場合、そのパケットの復号が
　　できなくなっていた問題を修正

　　http://www.marumo.ne.jp/db2008_2.htm#12 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.5.lzh

　・2008, 2/2 - ver. 0.1.4

　　ver. 0.1.3 での PMT 処理方法変更に問題があり、PMT が更新された
　　場合、それ以降で正常な処理が行えなくなっていたバグを修正

　　B-CAS カードとの通信でエラーが発生した場合のリトライ処理が機能
　　していなかったバグを修正

　　http://www.marumo.ne.jp/db2008_2.htm#2 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.4.lzh

　・2008, 2/1 - ver. 0.1.3

　　有料放送等で未契約状態の B-CAS カードを使った際に、鍵が取得で
　　きていないにもかかわらず、間違った鍵で復号をしていた問題に対処

　　鍵が取得できなかった ECM に関連付けられたストリームでは復号を
　　行わず、スクランブルフラグを残したまま入力を素通しする形に変更
　　鍵が取得できない ECM が存在する場合、終了時にチャネル番号と
　　B-CAS カードから取得できたエラー番号を警告メッセージとして表示
　　する形に変更

　　暗号化されていないプログラムで例外を発生させていたバグを修正

　　http://www.marumo.ne.jp/db2008_2.htm#1 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.3.lzh

　・2008, 1/11 - ver. 0.1.2

　　デジタル BS 放送等で、PAT に登録されているのに、ストリーム内で
　　PMT が一切出現しないことがある場合に対応

　　PMT 内の記述子領域 2 に CA_descriptor が存在する場合に対応する
　　ため arib_std_b25.c 内部での処理構造を変更

　　別プログラムと同時実行するためにスマートカードの排他制御指定を
　　変更

　　http://www.marumo.ne.jp/db2008_1.htm#11 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.2.lzh

　・2008, 1/7 - ver. 0.1.1

　　セクション (PAT/PMT/ECM 等) が複数の TS パケットに分割されている
　　場合に、正常に処理できなかったり、例外を発生をさせることがある
　　バグを修正

　　http://www.marumo.ne.jp/db2008_1.htm#7 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.1.lzh

　・2007, 11/25 - ver. 0.1.0

　　公開

　　http://www.marumo.ne.jp/db2007_b.htm#25 又は
　　http://www.marumo.ne.jp/junk/arib_std_b25-0.1.0.lzh

