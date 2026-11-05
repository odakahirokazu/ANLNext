# 3. パラメータとYAML

[目次](README.md) · [前：ライフサイクル](lifecycle.md) · [次：モジュール開発](module_development.md)

## 登録する値と型

モジュールは`mod_define()`で、Submoduleは`define_parameters()`でパラメータを登録します。
登録は、名前とC++メンバ変数を結び付ける操作です。
Ruby／Pythonから自由な型の項目を追加するのではなく、登録済みのC++型へ値を設定します。
初期値はC++メンバの初期値です。

主な型は次のとおりです。

| C++の値 | `type_name()`の例 | Ruby／Pythonの値 |
| --- | --- | --- |
| `bool` | `bool` | `true/false`／`True/False` |
| 符号付き整数型 | `int`／`integer` | Integer／int |
| `uintmax_t` | `unsigned_integer` | 非負のInteger／int |
| `double` | `double` | 数値 |
| `std::string` | `string` | String／str |
| `std::vector<int/double/std::string>` | `vector<int>`など | Array／list |
| scalarの`std::map` | `map<string,double>`など | Hash／dict |
| レコードのvector／map | `record_vector`／`record_map` | 専用の挿入APIで指定 |

scalar mapの組み合わせは、キーが`int`または`std::string`、
値が`int`・`double`・`std::string`の計6種類です。
vectorやmapは、登録された型に応じたsetterで変換します。
空のArray／listはvectorを空にできます。要素の型を先頭要素から推測する方式ではありません。

`define_result()`は、解析結果として公開する値の登録です。
同じRegistryではparameterとresultを合わせて名前を一意にします。
別のモジュールやSubmoduleでは同じ名前を使えます。
resultの登録だけで並列結果が集約されたり、毎runリセットされたりはしません。

## Chainから設定する

Ruby／PythonのChainは、次のAPIを持ちます。

| 対象 | 1項目 | 複数項目 |
| --- | --- | --- |
| 選択中のモジュール | `set_parameter(name, value)` | `set_parameters(parameters)` |
| module_idで指定 | `set_module_parameter(module_id, name, value)` | `set_module_parameters(module_id, parameters)` |

`with_parameters()`は`set_parameters()`の正式な別名です。
`chain()`で追加したモジュールが選択中になります。
module_id指定の設定APIも、そのモジュールを選択し、呼び出し後も選択状態を保持します。

```ruby
# processorというIDのモジュールを追加した後の断片
chain.with_parameters(input: 4.0)
chain.set_parameter('input', 5.0)
chain.set_module_parameter(:processor, 'input', 6.0)
```

```python
# processorというIDのモジュールを追加した後の断片
chain.with_parameters({'input': 4.0})
chain.set_parameter('input', 5.0)
chain.set_module_parameter('processor', 'input', 6.0)
```

いずれも設定を予約します。上の例では、適用後の`input`は`6.0`です。
定義後に呼んでも即時設定にはなりません。
通常は`run()`が適用します。設定途中に値を確認する場合は、定義後に`load_all_parameters()`を呼びます。

通常の値・配列・Hash／dictは、予約時の内容をコピーして保持します。
予約後に元の配列を書き換えても、その予約には反映されません。
`with_setter`などの任意のブロック／関数はクロージャのまま保持するため、実行時の外部状態を参照できます。

設定は登録順に1回ずつ適用します。値の代入は後のものが優先されます。
レコードの挿入は代入とは異なり、挿入操作を順番に実行します。
失敗した場合は例外を送出し、同じChainでの予約実行や解析の再開を拒否します。
適用済みの値を巻き戻したり、成功済みの設定を再実行したりはしません。

## 大きな値を即時設定したい場合

初期化開始後はChainへの新しい設定予約を認めません。
一方、モジュールを直接操作するAPIは即時設定です。
大量の配列を予約用にコピーしたくない場合などは、定義後に直接設定できます。

```python
# samplesがvector<double>として登録されたモジュールに対する断片
assert chain.define() == anlnext.ANLStatus_proceed
chain.load_all_parameters()
module = chain.get_module('processor')
anlnext.set_vector_parameter(module, 'samples', large_values)
```

Rubyでは`mod.set_parameter('samples', large_values)`を使えます。
直接設定でもSWIGでの変換やC++コンテナへのコピーは発生し得ます。
この方法で省くのは、設定予約のための値コピーです。
並行チェーンにも反映したい値は、複製前に設定してください。

## 符号なし整数と単位

`uintmax_t`では負値と範囲外の整数を受け付けません。
アドレスなどは整数値として扱い、浮動小数点を経由させないでください。

```python
# addressがuintmax_tとして登録された、定義済みモジュールに対する断片
module.set_parameter_unsigned_integer('address', 0x1000)
address = module.get_parameter('address').get_value_unsigned_integer()
```

Rubyでも`set_parameter_unsigned_integer()`を使用できます。
Chainの通常の整数設定は、登録された型を確認して符号なし整数のsetterへ振り分けます。

単位を登録したパラメータでは、設定・取得の値は登録単位で表します。
たとえば、内部単位を秒とし、係数`0.001`、単位名`ms`で登録したdoubleは、
設定値`5.0`を内部値`0.005`として保持します。表示・取得時は`5.0`です。
YAMLでもスクリプトと同じ登録単位を使います。

## レコードのvectorとmap

`record_vector`／`record_map`は、複数の名前付き値要素を持つレコードのコンテナです。
scalarのvector／mapとは別の型です。C++側で値要素の順序・名前・初期値を登録します。

```ruby
# rows、tableがそれぞれレコード型として登録された後の断片
chain.push_to_vector('rows', {count: 7, label: 'first'})
chain.insert_to_map('table', 'row1', {count: 11, label: 'second'})
```

```python
# rows、tableがそれぞれレコード型として登録された後の断片
chain.push_to_vector('rows', {'count': 7, 'label': 'first'})
chain.insert_to_map('table', 'row1', {'count': 11, 'label': 'second'})
```

省略した要素には、その時点の編集用バッファの値を使います。
通常の挿入後は初期値へ戻るため、各レコードでは必要な要素を明示すると意図が読みやすくなります。
Pythonでモジュールを直接操作する場合は、
`anlnext.push_to_vector(module, name, values)`／`anlnext.insert_to_map(module, name, key, values)`を使います。
Rubyはモジュール自身の同名メソッドを使えます。

## Submoduleの値

実体を選択した後、親と同じ`with_parameters()`で設定します。

```ruby
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters(input: 4.0, "algorithm.factor": 7.0)
# 同じ設定の別表記
chain.with_parameters(algorithm_: {factor: 7.0})
```

```python
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters({'input': 4.0, 'algorithm.factor': 7.0})
chain.with_parameters({'algorithm_': {'factor': 7.0}})
```

ドット形式は`Submodule名.パラメータ名`の2要素です。
一括設定では、ドットを含まず末尾が`_`のキーをSubmoduleのグループとして解釈します。
末尾の`_`を1個だけ取り除いたものが登録名です。
グループの中は値の設定であり、さらにSubmodule階層を掘る指定にはなりません。
scalar mapの内部キーにあるドットや`_`は、そのままキーとして扱います。

## YAMLファイル

第1章の例題を、次の`parameters.yaml`で設定できます。

```yaml
processor:
  input: 4.0
  algorithm_:
    factor: 7.0
```

親モジュールとSubmodule実体はスクリプトで選び、値だけをYAMLに記述します。
チェーンを構成し、Submodule選択を予約した後、`run()`の前に読み込みます。

```ruby
chain.load_parameters_from_yaml('parameters.yaml')
```

```python
chain.load_parameters_from_yaml('parameters.yaml')
```

最外層のキーは既存のmodule_id、各値はパラメータ名から値へのmappingです。
`algorithm.factor: 7.0`も使用できます。
ファイルは1個のYAML documentとし、`{}`は変更なし、空ファイル・null・最外層の配列はエラーです。
モジュールIDとパラメータ名は文字列にします。

YAMLの読み込みも設定予約です。前後の`with_parameters()`と呼び出し順に並び、後の代入が優先されます。
ファイル読込時に構造とmodule_idを確認し、未知のパラメータや値の型違反は適用時に検出します。
YAMLは既存の値設定APIを使用し、レコード型専用の新しい表現は追加していません。

整数mapのキーは整数で、文字列mapのキーは文字列で書きます。
`"on"`、`"off"`、日付風の文字列、数字からなる文字列などは引用符で囲んでください。
重複キーは避け、浮動小数点の指数表記には`1.0e+3`のような形を使います。
詳細は[YAML仕様](../design/yaml_parameters_v3.md)を参照してください。

## 表示・値取得・保存

設定を適用してから取得します。resultも同じ方法で取得できます。

```ruby
mod = chain.get_module(:processor)
mod.print_parameters
value = mod.get_parameter_value('input')
algorithm = mod.get_submodule('algorithm')
factor = algorithm.get_parameter_value('factor')
```

```python
module = chain.get_module('processor')
module.print_parameters()
value = anlnext.get_value(module.get_parameter('input'))
algorithm = module.get_submodule('algorithm')
factor = anlnext.get_value(algorithm.get_parameter('factor'))
```

表示・値取得・JSON出力は、レコードの編集用バッファを書き換えません。
マネージャーの`parameters_to_json(filename)`は、定義情報やSubmoduleも含む設定情報を出力します。
このJSONはYAML入力ファイルとは別の形式です。
低水準の`value_elements_from_container()`は所有権を持つラッパーを返す補助APIです。
通常は上の値取得APIを使い、`__deref__`を手動で呼ぶ必要はありません。
