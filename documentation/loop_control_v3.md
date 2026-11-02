# Version 3: ループ制御

## 対象

REDOの廃止と、並列実行時のOrderKeeperの終了契約を扱う。
パラメータ登録・複製、Submodule、Module間の依存宣言は別の変更とする。

## 試行回数と成果数

`run_analysis(n)`のnは、チェーン処理を試みる回数の上限とする。
各試行には0から始まる異なるloop番号を割り当てる。
SKIPも一試行として消費し、同じ番号での再実行は行わない。
`AS_REDO`と`ANLStatus::redo`は削除した。
列挙値の数値も変わるため、利用側のC++ライブラリとバインディングを再ビルドする。

必要な成果数に達したかはModuleが判断する。
MTで全体を止める場合はQUIT_ALLを使う。処理中の反復が残るため、
成果を厳密に指定件数へ制限する責任は採用・出力を担当するModuleにある。

## Loop番号の名前と型

フレームワークの処理単位を表す名前は、扱うデータの種類に依存しないloopに統一する。
処理対象の`loop_index`とModuleの`get_loop_index()`は、同じ`std::size_t`の値を扱う。
OrderKeeperと例外情報もこの型を使う。
まだ番号が設定されていないModule、および複製直後のModuleでは、
`BasicModule::undefined_loop_index`（`std::size_t`の最大値）を返す。
有効な番号は指定回数未満なので、最大回数を指定した場合もこの未設定値とは重ならない。

MTの`next_loop_index_`は、次に割り当てる番号であり、各ワーカーの処理中の番号とは区別する。
`process_one_event()`は`process_one_iteration()`、`event_index_to_process()`は`next_loop_index()`、
`print_event_index()`は`print_loop_index()`、`ANLRequest::show_event_index`は`show_loop_index`へ改名した。
物理イベントを扱う例題や、Evsのevent selectionという用語は維持する。

## 順序保証

順序依存Moduleのmod_analyzeはloop番号順に実行する。
それ以外のModuleは並列実行できる。
SKIP後や無効Moduleでも順序制御点を通過し、後続の処理を取り残さない。

OrderKeeperは次に通過する番号をsize_tで保持する。
KeeperBlockは、待機が正常に完了した場合だけ通過完了を通知する。
コピーは禁止し、同じ通過を二度通知しない。

## 通常終了と異常終了

- QUITはそのワーカーを終了する。他のワーカーは継続する。
- QUIT_ALLとコンソールの終了要求は、新しい反復の割り当てを止める。
- 通常終了では、割り当て済みの処理は残りの順序制御点を通過する。
- 例外では後続の制御点への到達が保証できないため、全OrderKeeperをcancelする。
- cancelされたwaitはfalseを返し、待機していた側はModuleを実行せず終了する。
- critical statusを処理したワーカーも全体に停止・cancelを通知する。
- cancel以前に実行を開始したModuleを強制中断したり、副作用を巻き戻したりはしない。
- ワーカー生成途中で失敗した場合も、cancel後に生成済みワーカーをjoinする。

例外伝播を無効にした場合のプロセス終了、およびhard_terminateは既存の方針を維持する。

モジュール結果の集約が非OKを返した場合、`run_analysis()`はそのステータスを返す。
統計の集約と表示は従来どおり実施する。

## 結果集約の引数

`mod_reduce()`の引数は`const std::vector<BasicModule*>&`とする。
master自身を除く各並列チェーンの、同じ位置のModuleをチェーン順に渡す。
デフォルト実装は順に`mod_merge(const BasicModule*)`を呼び、非OKが返れば集約を中断する。
`mod_reduce()`をoverrideする利用側は引数型を変更する。`mod_merge()`のAPIは維持する。

## SIGINTの設定

`initialize_analysis()`・`run_analysis()`・`finalize_analysis()`で一時変更するSIGINTの設定は、
正常終了・非OKステータス・例外のいずれでも呼び出し前の状態に戻す。
ハンドラだけでなく、マスクとフラグも復元する。呼び出し元へ送出される例外は元のまま伝播させる。
設定・通常終了時の復元に失敗した場合は、従来どおりcriticalステータスを返す。
例外処理中の復元に失敗した場合は、診断を出力し、元の例外の伝播を維持する。

## 解析中のコンソール

コンソール有効時も、解析の例外はコンソールスレッドへ終了を通知してjoinした後、
呼び出し元へ再送出する。入力がない場合や、改行前の入力がある場合でも終了できる。
`select()`で入力を待ち、届いた文字を`read()`で蓄積してコマンドを処理する。
終了確認の待ち時間は最大約1秒とし、EOFでもコンソールを終了する。

`.q`・`.i`・`.s`と末尾の空白の扱いは維持する。
この簡易コンソールではreadline固有の編集・履歴機能を使わない。
パラメータ設定用の対話入力は従来のままとする。

## 複数回のrun

`define_analysis()`で定義し、`pre_initialize_analysis()`で複製前の初期化を行い、
その末尾で並列チェーンを複製する。`initialize_analysis()`で各チェーンを初期化した後は、
同じManager・同じModuleで`run_analysis()`を繰り返せる。
最後に`finalize_analysis()`を一度呼ぶ。各runでは再定義・再複製・再初期化を行わない。

正常時のrunは次の順序で実行する。

1. 全チェーンのフレームワークの統計・Evsカウント・フラグ・実行制御をリセットする。
2. 各チェーンの`mod_begin_run()`を呼ぶ。
3. 解析ループを実行する。
4. 各チェーンの`mod_end_run()`を呼ぶ。
5. 並列Moduleの結果を`mod_reduce()`で集約する。
6. フレームワークの統計とEvsを集約し、表示する。

管理側のrun準備は内部メソッド`prepare_run()`にまとめ、
`routine_*()`は`routine_modfn()`によるModuleメソッドの呼び出しと、MTでの各チェーンへの適用を担当する。
`prepare_run()`は新しいModule側のライフサイクルメソッドではない。

処理統計とEvsは各runについて集計し、以前のrunの値は加算しない。
反復回数が0でもbegin/end/reduceは実行し、フレームワークのカウントは0とする。
Module固有の状態・資源・入力位置・乱数状態・解析結果は自動リセットしない。
継続・初期化・累積の扱いはModuleが`mod_begin_run()`・`mod_end_run()`・`mod_reduce()`等で規定する。
結果を累積する場合も、過去の寄与を再加算しない責任はModule側にある。
`histogram_mt`の`FillHistogram`は、各runの`mod_begin_run()`でヒストグラムをリセットする例とする。

## Run番号

Managerがloop番号と同じ`std::size_t`のrun番号を保持し、`run_index()`で取得する。
初期化までの値は`ANLManager::undefined_run_index`（型の最大値）、最初のrunは0とし、run開始時に更新する。
Moduleでも同じ値の`BasicModule::undefined_run_index`を使う。
`mod_begin_run()`より前に全チェーンのModuleへ同じ番号を渡し、Moduleは`get_run_index()`で取得する。
番号は正常完了数ではなく開始したrunを識別するため、begin_runの失敗でも消費する。
finalize中も最後のrun番号を維持する。複製直後のModuleのrun番号も未設定値とする。
Ruby/PythonでもManagerの`run_index()`、Moduleの`get_run_index()`と`get_loop_index()`、
および各クラスの未設定値の定数を利用できる。

各runのloop番号は0から開始し、begin_runの時点では`undefined_loop_index`とする。
MTで次に割り当てる番号とOrderKeeperも、前回のワーカーが終了した後、begin_runより前にリセットする。

critical statusや送出された例外からの復旧・再runは今回のサポート対象に含めない。
既存のfinalize/terminate指定と例外伝播の方針を維持する。
既存の実行制御テストで異常終了後に再実行するのは、制御状態のリセットに限った確認とする。

Ruby/Pythonの上位APIである`AnalysisChain.run()`は、準備からfinalizeまでをまとめる従来の動作を維持する。
複数runは、初期化済みの`ANLManager`/`ANLManagerMT`の`run_analysis()`を繰り返し呼び出す。

## 検証

CTestにタイムアウト付きの回帰テストを登録した。
逐次実行、並列順序、SKIP、無効な順序依存Module、QUIT、QUIT_ALL、
標準例外、ANL例外のfinalize/terminate指定、critical statusを確認する。
各経路の後に同じManagerで再実行し、loop番号と順序制御の再開も確認する。
正常時の複数runについては別のテストで統計とModuleの結果も確認する。
例外テストでは、先頭の反復が例外を送出する前に別の反復の開始を待つ。
コンソールのテストではパイプの書き込み側を開いたままにし、入力なし・入力途中の終了を確認する。
単一・並列実行の例外伝播、3コマンド、EOFと改行のない最終コマンドも確認する。
SIGINTについては、3つの処理段階それぞれの正常終了・criticalステータス・標準例外・ANL例外で
設定の復元を確認する。通常のハンドラ・SIG_IGN・SA_SIGINFOと、マスク・フラグを検証し、
実際のシグナル送信は行わない。
Evsのカウントリセットでは、`counts`と`counts_ok`の両方がゼロになり、
定義とフラグが維持され、ゼロから再集計できることを確認する。
Loop番号は符号付き整数の上限を超える値も含め、逐次・並列用の1反復処理でModuleへの受け渡しと
例外情報・表示への格納、複製時の未設定値への初期化を確認する。

逐次・MTの1チェーン・MTの複数チェーンについて、異なる回数と0回のrunを繰り返す。
run番号、loop番号、各段階の呼び出し順、各runの統計・Evs、Moduleの実体と状態の維持を確認する。
Moduleが結果を毎runリセットする場合と累積する場合を検証し、
begin_runのcritical statusと例外でも、全チェーンのrun番号更新と統計リセットが先に完了することを確認する。
Ruby/Pythonからの複数runと番号取得も検証する。

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
