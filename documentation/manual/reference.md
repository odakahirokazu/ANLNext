# 7. ステータス・移行・問題の切り分け

[目次](README.md) · [前：並列実行](parallel.md)

## mod_analyze()の戻り値

| 正規の定数 | `ANLStatus`の値 | 動作 |
| --- | --- | --- |
| `AS_PROCEED` | `proceed` | 次のモジュールへ進む |
| `AS_PROCEED_WITH_ERROR` | `proceed_with_error` | エラーを集計し、次へ進む |
| `AS_SKIP` | `skip` | 現在の反復の残りを飛ばす |
| `AS_SKIP_WITH_ERROR` | `skip_with_error` | エラーを集計してSKIP |
| `AS_QUIT` | `quit` | 自チェーンのrunを終了 |
| `AS_QUIT_WITH_ERROR` | `quit_with_error` | エラーを集計してQUIT |
| `AS_QUIT_ALL` | `quit_all` | 全チェーンにrunの終了を要求 |
| `AS_QUIT_ALL_WITH_ERROR` | `quit_all_with_error` | エラーを集計してQUIT_ALL |

`AS_`はANL Statusの略です。
`AS_OK`は全言語で`AS_PROCEED`の正式な別名として残し、非推奨にはしません。
`status_to_string(AS_OK)`は正規名の`AS_PROCEED`を返します。

`WITH_ERROR`はエラーの集計を付加し、対応するエラーなしの制御と同じ動作をします。
`AS_PROCEED_WITH_ERROR`では、そのモジュールのerrorとokの両方が増えます。
集計後に制御用のエラーなしステータスへ変換するため、通常エラーがrun全体の戻り値に残る仕様ではありません。
また、これらの名前だけから、ユーザーデータの整合性や部分的な結果の有効性が保証されるわけではありません。

単一チェーンではQUITでrunが終わります。
並列実行で全体を終える場合はQUIT_ALLを使います。
既に割り当て済みの反復が残るため、厳密な成果数の上限は結果を採用するモジュールで管理してください。

## criticalステータスと例外

| 値 | 意図 |
| --- | --- |
| `critical_error_to_finalize` | 後処理を行う方針で解析を中断 |
| `critical_error_to_terminate` | 通常の後処理を続けない方針で中断 |
| `critical_error_to_finalize_from_exception` | 例外をfinalize方針で扱ったことを示す |
| `critical_error_to_terminate_from_exception` | 例外をterminate方針で扱ったことを示す |

先頭2個には`AS_CRITICAL_ERROR_TO_FINALIZE`／`AS_CRITICAL_ERROR_TO_TERMINATE`を用意します。
`from_exception`の2個には短縮定数を追加していません。
criticalの意図を受けて実際にfinalizeを呼ぶかどうかは、実行を制御する呼び出し側も確認します。
低水準のManager APIを直接呼ぶ場合は、戻り値を確認して次の処理を決めてください。

例外処理も設定失敗の扱いも、自動復旧の仕組みではありません。
Ruby／Pythonの高水準Chainは、例外が発生した設定処理を再開しません。
エラー処理の詳細は[ループ制御](../design/loop_control_v3.md)を参照してください。

## 各言語の名前

| 用途 | C++ | Ruby | Python |
| --- | --- | --- | --- |
| 列挙値 | `anlnext::ANLStatus::proceed` | `ANL::ANLStatus_proceed` | `anlnext.ANLStatus_proceed` |
| 利用者向け定数 | `anlnext::AS_PROCEED` | `ANL::AS_PROCEED` | `anlnext.status.AS_PROCEED` |
| 正式な別名 | `anlnext::AS_OK` | `ANL::AS_OK` | `anlnext.status.AS_OK` |

Ruby／Pythonの内部処理は列挙値を直接参照します。
SWIGの`ANLStatus_ok`などの旧列挙名は残しません。

## version 2からの主な移行点

利用側のC++ライブラリとバインディングを再ビルドしてください。
旧APIをすべて維持する方針ではありません。

| 旧API・挙動 | version 3で使うもの |
| --- | --- |
| Managerの`Define()` | `define_analysis()` |
| `PreInitialize()` | `pre_initialize_analysis()` |
| `Initialize()` | `initialize_analysis()` |
| `Analyze()` | `run_analysis()` |
| `Finalize()` | `finalize_analysis()` |
| `AS_ERROR` | `AS_PROCEED_WITH_ERROR` |
| `AS_SKIP_ERROR`など | 対応する`*_WITH_ERROR` |
| `ANLStatus::ok`などの旧列挙名 | `ANLStatus::proceed`などの正規名 |
| REDO | 廃止。成果数を数え、必要な時点でQUIT／QUIT_ALL |
| `get_module_NC()` | `get_module_non_const()` |
| `get_module_IF()` | `get_module_interface()` |
| `get_module_IFNC()` | `get_module_interface_non_const()` |
| `mod_reduce()`の`std::list`引数 | `const std::vector<BasicModule*>&` |
| Rubyの`set_parameters(module_id, parameters)` | `set_module_parameters(module_id, parameters)` |
| Pythonのモジュールに対するレコード挿入 | `anlnext.insert_to_map()`／`anlnext.push_to_vector()` |
| `CreateSwigRuby.cmake`／`CreateSwigPython.cmake` | `ANLNextSwigRuby.cmake`／`ANLNextSwigPython.cmake`の`anlnext_add_swig_*_binding()` |

C++の旧エラー定数とModuleAccessの旧メソッド名は、移行用にdeprecatedな別名を残します。
Ruby／Pythonでは旧エラー定数を追加しません。
`request_module*()`の命名も同じ変更です。

設定予約はRuby／Pythonで共通です。定義後のChain API呼び出しも予約になり、
表示・値取得は予約を暗黙に適用しません。
詳細な一覧は[APIの命名と移行](../design/naming_v3.md)を参照してください。

## よくある問題

| 症状 | 確認すること |
| --- | --- |
| `require`／`import`で見つからない | インストール先、`RUBYLIB`／`PYTHONPATH`、例題のバインディングもインストールしたか |
| 拡張ライブラリの読み込みに失敗する | インタプリタとビルド時の開発ライブラリの対応、共有ライブラリの探索先、再ビルド漏れ |
| PythonでYAMLだけ読めない | 実行中のPython環境にPyYAMLがあるか |
| 設定した値がまだ見えない | 定義後に`load_all_parameters()`を実行したか |
| Submoduleが未設定 | 名前を宣言し、値設定より先に実体選択を予約したか |
| パラメータが重複登録になる | 同じRegistry内でparameterとresultの名前が重複していないか |
| mapの値がSubmoduleとして解釈される | 一括設定のパラメータ名が末尾`_`のグループ形式になっていないか |
| モジュール要求エラー | module_id／別名、前後関係、privacyによる非登録を確認 |
| 設定失敗後に再実行できない | 同じChainで再開せず再構築。Rubyでは明示的な`clear()`も可能 |
| 同じChainで2回目の`run()`ができない | 複数runは初期化済みManagerの`run_analysis()`で行う |
| 並列で結果がおかしい | ポインタの共有、コピー設計、順序依存、集約と毎runのリセットを確認 |

## コンソールとAPIリファレンス

解析中のコンソールを有効にすると、`.q`でrunの終了要求、`.i`でloop番号、`.s`でEvs集計を表示できます。
高水準Chainの`console`は初期値がtrue、低水準`run_analysis()`の引数は初期値がfalseです。
パラメータ設定用のinteractive modeと解析中の簡易コンソールは別の機能です。
interactive modeの全面的な再設計は今回の対象に含めていません。

C++ APIリファレンスは、Doxygenを利用できる環境で次のように生成できます。

```sh
cd documentation/doxygen
doxygen Doxyfile
```

既存設定では、`documentation/html/doxygen/index.html`がHTMLの入口です。
引数型や公開範囲は、マニュアルに加えて各クラスのヘッダとAPIリファレンスで確認してください。
