# 単純なキー・値を持つmapパラメータ

`std::map<K, V>`のうち、Kが`int`または`std::string`、Vが`int`、`double`、
`std::string`の6通りを、vector<int>などと同様の値型パラメータとして扱います。
`ModuleParameter_record_map.hh`が扱う表形式のmapとは別の特殊化です。
これらの6通りに`add_value_element()`を使うとエラーになります。
従来の`map<string,int>`などを表形式として使っていた場合は、
`map<string,tuple<int>>`などに変更して値要素の登録を維持できます。

```cpp
std::map<int, double> coefficients_;
define_parameter("coefficients", &mod_class::coefficients_);
set_parameter("coefficients", std::map<int, double>{{1, 0.5}, {2, 1.5}});
auto values = get_parameter("coefficients")->get_value(std::map<int, double>{});
```

設定はmap全体の置き換えです。空のmapを渡すと空になります。
`double`の値は設定時にunitを掛け、取得・表示・JSON出力時に割ります。
キーにはunitを適用しません。複製時は各モジュールが独立したmapを持ちます。

RubyのChainでは、定義後に型を調べてHashの設定方法を選びます。
従来の表形式の`map`には、引き続き既存の要素登録処理を使います。

```ruby
chain.with_parameters(coefficients: {1 => 0.5, 2 => 1.5})
# Define後に直接設定する場合:
mod.set_parameter('coefficients', {1 => 0.5})
values = mod.get_parameter_value('coefficients') # Hash
```

Pythonでも、Chainは定義後に型を調べます。

```python
chain.with_parameters({'coefficients': {1: 0.5, 2: 1.5}})
# Define後に直接設定する場合:
anlnext.set_map_parameter(module, 'coefficients', {1: 0.5})
values = anlnext.get_value(module.get_parameter('coefficients')) # dict
```

bindingの型を明示するAPIは`set_parameter_map_i_i`、`set_parameter_map_i_d`、
`set_parameter_map_i_str`、`set_parameter_map_str_i`、`set_parameter_map_str_d`、
`set_parameter_map_str_str`です。取得は対応する`get_value_map_*({})`です。
空のHash/dictも登録済みの型に基づいて設定します。
型変換と`int`の範囲確認には既存bindingと同じSWIGの変換を使います。
文字列キーと整数キーは相互に変換しません。

対話入力は`key value key value ...`形式です。文字列は引用符で囲めるので、
空文字列や空白・引用符・バックスラッシュを含む値も指定できます。
同じキーが複数回あれば最後の値を採用します。不完全な組や範囲外の整数など、
入力が不正なら更新しません。対話操作で空にする指定はvectorと同じ`;`です。

JSONの型名は`map<int,double>`などです。`value`は`key`と`value`を持つ
要素の配列で出力します。整数キーをJSONオブジェクトの文字列キーへ変える
ことはしません。C++の数値が文字列になる点や、空のコンテナが`""`になる点は、
既存のBoost.PropertyTreeによる出力と同じです。Rubyの出力では数値の型を保ち、
空のコンテナは`[]`です。

```sh
ctest --test-dir <build-directory> -R map_parameter --output-on-failure
```

表形式の型名は`record_map`／`record_vector`です。通常のコンテナは
`map<int,double>`／`vector<int>`など、キーや値の型を含む名前を使います。
