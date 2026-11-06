# Version 3: パラメータ管理の検討事項

## 登録名の一意性

同一の`ParameterRegistry`内では、parameterとresultを合わせて登録名を一意とする。
`define_parameter()`、`define_result()`で既存の名前を再登録すると、
`ParameterAlreadyDefinedError`を送出し、既存の登録と選択状態を維持する。
明示的に`undefine_parameter()`で削除した後の再定義は認める。
モジュールやSubmoduleなど、別々のRegistry間では同じ名前を使える。

## レコードの参照・出力

`record_vector`と`record_map`の表示・JSON出力は、格納済みレコードを読み取り、
編集用バッファや値要素が参照するメンバ変数を書き換えない。
出力後に編集・挿入を継続しても、編集中の値と初期値は維持される。
出力が例外で中断した場合にも、この状態を維持する。

以前は`output()`・`to_property_tree()`で各レコードを編集用バッファへ取り出し、
最後のレコードの値が残っていた。設定完了後に出力する運用では問題が表面化しなかったが、
出力後の挿入値や初回の初期値確定に影響するため、読み取りと編集を分離した。

### 読み取り用API

`VModuleParameter::value_elements_from_container(key/index)`は、
指定レコードの値と定義情報を持つ独立した値要素のvectorを返す。
返された値要素は自身で値を所有し、変更しても元のコンテナ・編集用バッファには影響しない。
名前・型・単位などの情報は、登録済みの定義から引き継ぐ。
メンバ変数を使う値要素も、返された側では元のメンバ変数を参照しない。

既存の`retrieve_from_container()`は、編集のためにレコードをバッファへ取り出すAPIとして維持する。
`value_element_info()`も、登録済みの定義と編集用の値を参照する従来の動作を維持する。

### 実装とバインディング

値を所有する既存の`ModuleParameterUnique`に、定義情報と値を受け取るコンストラクタを追加した。
scalar・pair・tupleから独立した値要素を作る処理を共通化し、表示・JSON・読み取り用APIで使用する。
従来の表示形式とJSON構造、単位換算、record mapの条件付き要素表示を維持する。
条件付き要素表示は編集中の値ではなく、出力対象のレコードのselectorを使う。
JSONは従来どおり無効化された要素も含める。

Rubyの`to_value_object()`・`to_object()`、Pythonの`get_value()`も読み取り用APIを使う。
これにより、Rubyの`parameters_to_object()`やスクリプト生成を含む参照・保存が、
後続のレコード編集・挿入に影響しなくなる。

Rubyでは、返された`shared_ptr`のラッパー`SPtrModParam`にも
`to_object()`・`to_value_object()`を用意し、`VModuleParameter`の実装へ委譲する。
呼び出し側は`__deref__`せず、所有権を持つラッパーのまま値や定義情報を取得できる。
Pythonの値取得も、`shared_ptr`のラッパーに公開されたメソッドを直接使う。

### 検証

所有値とメンバ変数を使う値要素について、表示・JSON出力・出力途中の例外の前後で
編集用の値が維持され、挿入値・初期値が変わらないことを確認する。
scalar・pair・tuple、bool・unsigned integer・vector・mapの値要素、単位、条件付き表示を扱う。
読み取りで得た値要素と、その複製が独立した値を持つことも確認する。
Ruby/Pythonでも、編集中に表示・値取得・JSON保存を行ってから挿入する回帰テストを追加した。
返されたvectorから取り出した要素は、vectorがGCで回収された後も利用できることを確認する。

## Submoduleへの参照とパラメータ設定

`define_submodule(name, &ModuleClass::member)`は、登録名と参照用のポインタメンバを結び付ける。
実体の所有権は親の登録表にあり、メンバは宣言時には`nullptr`、登録・交換後には登録済み実体を指す。
親とSubmoduleの複製後は、複製先の親のメンバを、そのチェーンの実体へ結び直す。
実体の型はメンバの指す型へ変換可能である必要があり、不一致は`SubModuleBindingError`とする。
名前だけの宣言も維持し、結び付けていないユーザーの参照は自動更新しない。

登録情報は、実装用クラス`SubModuleEntry`が名前と実体の所有権を保持する。
メンバ付き登録には、その派生クラス`SubModuleMember`を使い、メンバ変数ポインタを直接保持する。
実体の登録・複製の共通処理は基底クラス、型の確認とメンバへの代入は派生クラスが担当する。
複製では、まず登録情報を同じ型でコピーし、実体を複製して新しい親へ接続する。
元の親へのポインタや任意の関数は保持しない。
ユーザーは`define_submodule()`を使い、これらのクラスを直接生成する必要はない。

Ruby/Pythonでは、`chain()`、`with_submodule(name, key)`、`with_parameters()`の順に設定する。
`with_parameters()`は親の通常名と`submodule.parameter`形式の名前を受け付ける。
この形式はSubmodule名とパラメータ名の2要素とし、空の要素や追加の階層はエラーにする。
通常のパラメータ名のアンダースコアは区切りにしない。mapの値のキーも階層として解釈しない。
Ruby/Pythonとも、Chain経由の設定は定義の前後を問わず予約する。
実体の選択は値設定より先に行い、未宣言・未設定の実体や未知のパラメータは既存の例外とする。

## Chainの設定予約と実行

設定APIは、対象の指定方法と項目数で区別する。

| 対象 | 1項目 | 複数項目 |
|---|---|---|
| 選択中のモジュール | `set_parameter(name, value)` | `set_parameters(parameters)` |
| module_idで指定 | `set_module_parameter(module_id, name, value)` | `set_module_parameters(module_id, parameters)` |

`with_parameters()`は`set_parameters()`の正式な別名として維持する。
Rubyではブロック付きの呼び出しと、`ANLApp`・`ModuleInitializer`経由の設定も使える。
設定処理は両言語で`apply_setting()`を使って予約できる。Rubyの`apply_setting`は`set_parameters`の別名とする。
module_id指定版は対象モジュールを選択して設定を予約し、その後も選択状態を保持する。
以前のRubyの`set_parameters(module_id, parameters)`は`set_module_parameters()`へ改名した。

`with_parameters()`、YAMLからの値設定、Submodule選択、レコードの挿入、
`apply_setting()`による設定処理は、登録順に並ぶ一つの予約リストへ入れる。
同じ名前の設定も省略せず、各操作を順番に実行する。
通常の値設定は後の設定が優先され、レコードの追加・挿入は既存の操作の意味に従う。
モジュールを直接操作する`mod.set_parameter()`などは即時設定とする。

通常の`run()`では、次の二箇所で予約をすべて実行する。

1. 定義後、`pre_initialize_analysis()`とチェーン複製の前。
2. 複製後の`modify`終了後、`initialize_analysis()`の前。

手動で`define()`、`load_all_parameters()`を呼んだ後も、追加した予約を`run()`が反映する。
定義済みのChainでは同じマネージャーを使い、定義を繰り返さない。
`load_all_parameters()`は定義後の設定可能な段階で呼ぶ。
予約が空なら値も解析の進行段階も変更しない。
設定予約の有無を「設定読み込み済み」という解析段階で代用しない。

表示・値取得・出力は適用済みの値を読み取り、予約を暗黙に実行しない。
`modify`内でも新しい設定は予約されるため、その場で設定後の値を参照する場合は
明示的に`load_all_parameters()`を呼ぶ。
初期化開始以降のChain経由の予約はエラーとし、未実行の予約を残さない。

成功した予約は実行後に消化する。予約の実行中に追加された操作も末尾から順に処理する。
予約の実行処理への再入は認めない。
実行が例外で失敗した場合は呼び出し元へ送出し、以後、そのChainでの設定・予約実行・解析を拒否する。
成功済み操作を再実行せず、失敗した操作と後続操作も自動で再試行しない。
適用済みの値は巻き戻さない。新しいChainを構築し直すか、Rubyでは明示的に`clear()`して再構築する。

Rubyのinteractive経路でも、対話入力前に予約を反映し、対話入力中に追加された予約は
チェーン複製前に反映する。対話モード自体の再設計は別途検討する。

Array/listのsetterは、Ruby/Pythonとも、実行時に登録済みのC++パラメータ型から選ぶ。
`vector<double>`には整数のみの配列や整数・小数の混在を設定でき、空配列も登録型に従って扱う。
要素の型・数値範囲の検査には、既存のSWIG setterを使う。

### 予約時の値の保持

通常の値設定では、設定APIを呼んだ時点の値をコピーして保持する。
元の配列・Hash/dictなどを変更しても、すでに予約した内容は変わらない。
同じオブジェクトを変更しながら複数回予約した場合は、それぞれの時点の値を使う。

Array/list・Hash/dictは中の値までコピーする。Rubyの文字列と`ANL::Vector`も独立した値にする。
数値・boolなどの不変値はそのまま保持する。
パラメータ名、record mapのキー、Submodule選択名・キーも予約時点で固定する。
通常のモジュールとSubmodule、単一・複数項目の設定、recordの追加で同じ方針を使う。

Rubyの`ModuleInitializer`も、最初に設定APIを呼んだ時点で値をコピーする。
Chainへ登録する前に元の値を変更しても、その影響を受けない。
Rubyは対応するスクリプトの値を再帰的にコピーし、Pythonは標準の`copy.deepcopy()`を使う。

任意の設定用ブロック・関数や`modify`は、関数として保持する。
クロージャが参照する外部状態は固定せず、実行時の値を使う。
実行時に設定値を決めたい場合は、`apply_setting()`に渡すブロックや関数内で値を設定する。

## Interactive modeの位置づけ

元々はinteractive modeによる対話入力が主流であり、編集用バッファを中心とする
パラメータ管理にはその背景がある。現在はinteractive modeをほとんど使用していないため、
今後の扱いを別途検討する。現時点で廃止や仕様変更を決めるものではない。

検討時には、対話入力に必要な編集状態と、通常のパラメータ設定・参照・出力との
関係を整理する。参照・表示・保存が後続処理の挙動を変えないことを設計上の指針とし、
対話入力の都合で共有してきた状態をどこまで共有する必要があるかを見直す。
