# uintmax_tパラメータ

単一の`uintmax_t`を`unsigned_integer`型のパラメータとして登録できます。
`0`から`UINTMAX_MAX`まで、符号付き整数や浮動小数点数への変換を通さず扱います。

```cpp
uintmax_t address_ = 0;
define_parameter("address", &mod_class::address_);
set_parameter_expression("hex"); // 必要なら16進数で表示・入力

set_parameter_unsigned_integer("address", std::numeric_limits<uintmax_t>::max());
auto value = get_parameter("address")->get_value_unsigned_integer();
```

C++では、`uintmax_t`を渡す`set_parameter(name, value)`と
`get_value(uintmax_t{})`も利用できます。
既存の`int`／`intmax_t` APIで設定する場合、負数を拒否します。
取得時は戻り値の範囲を超える値をエラーにし、切り詰めません。
引数が`uintmax_t`のAPIでは、呼び出し前のC++の暗黙変換に注意してください。
例えばC++の`set_parameter_unsigned_integer(name, -1)`はAPIに入る前に
符号なしに変換されます。符号付きの入力には`set_parameter_integer()`を使います。
符号なし専用APIを別の型のパラメータに使うと型エラーになります。

RubyのChainは、定義後のパラメータ型を調べて専用APIを選択します。

```ruby
chain.with_parameters(address: 0xffff_ffff_ffff_ffff)

# Define後の直接操作
mod.set_parameter_unsigned_integer('address', 0xffff_ffff_ffff_ffff)
value = mod.get_parameter('address').get_value_unsigned_integer
# ライブラリを読み込んでいる場合はこちらも利用可能
mod.set_parameter('address', 0xffff_ffff_ffff_ffff)
value = mod.get_parameter_value('address')
```

PythonでもChainがパラメータ型を確認して設定します。

```python
chain.with_parameters({'address': 0xffff_ffff_ffff_ffff})

# Define後の直接操作
module.set_parameter_unsigned_integer('address', 0xffff_ffff_ffff_ffff)
value = module.get_parameter('address').get_value_unsigned_integer()
value = anlnext.get_value(module.get_parameter('address'))
```

Ruby／Pythonからの専用APIでは、負数・上限超過・型違いを拒否します。
値を保存するC++型の上限は`std::numeric_limits<uintmax_t>::max()`です。
上の例の16進数リテラルは64ビットの全ビットを立てた値です。

対話入力は通常10進数で、`hex`指定時は16進数です。
16進数では`0x`／`0X`接頭辞を付けても省略しても入力できます。
負の符号、小数、不正文字、上限超過がある場合は更新せず入力エラーにします。

JSONの`type`は`unsigned_integer`で、`value`は10進数です。
`hex`は表示・対話入力にだけ影響します。
既存の出力規則に従い、C++のBoost.PropertyTreeでは数値文字列、
Rubyの`parameters_to_object()`では整数として記録します。
Rubyのスクリプト生成でも、16進数表示から文字列を転用せず整数値を出力します。

```sh
ctest --test-dir <build-directory> -R unsigned_integer --output-on-failure
```
