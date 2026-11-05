# 1. 導入と最初の解析

[目次](README.md) · [次：ライフサイクル](lifecycle.md)

## 必要なソフトウェア

現在のCMake設定は次を前提としています。

| ソフトウェア | 要件・用途 |
| --- | --- |
| C++コンパイラ | C++17 |
| CMake | 3.30以上 |
| Boost | 1.80.0以上 |
| SWIG | Ruby／Pythonバインディングの生成 |
| Ruby | 3.0以上。開発用ヘッダとライブラリも必要 |
| Python | Python 3。開発用ヘッダとライブラリも必要 |
| readlineまたはlibedit | コンソール・対話処理 |
| PyYAML | PythonでYAMLを読む場合と、YAMLを含むCTestの実行時 |

現状の本体ビルドではRubyとPythonの両バインディングを構築します。
SWIGとPythonの最低バージョンはCMakeで固定していません。
使用するインタプリタと、その開発用ライブラリを対応させてください。

ROOT、CLHEP、Geant4との連携は任意です。
この章の例題では必要ありません。YAMLを使わないPython実行にもPyYAMLは不要です。

## 本体のビルド

以下はANLNextリポジトリのルートで実行します。
インストール先は例として`$HOME/local/anlnext-v3`としています。

```sh
export ANLNEXT_INSTALL="$HOME/local/anlnext-v3"
cmake -S . -B build-v3 \
  -DCMAKE_INSTALL_PREFIX="$ANLNEXT_INSTALL" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF
cmake --build build-v3 --parallel 4
cmake --install build-v3
```

依存ライブラリが標準の探索先にない場合は、`CMAKE_PREFIX_PATH`でそのインストール先を指定します。
Pythonを明示する場合は`-DPython3_EXECUTABLE=/path/to/python`を追加します。
Rubyは`-DRuby_EXECUTABLE=/path/to/ruby`で指定できます。

主なオプションは次のとおりです。

| オプション | 初期値 | 内容 |
| --- | --- | --- |
| `ANLNEXT_ENABLE_INTERACTIVE_MODE` | `ON` | パラメータ設定などの対話モード |
| `ANLNEXT_USE_READLINE` | `ON` | readlineを使用。`OFF`ではlibeditにリンク |
| `ANLNEXT_USE_TVECTOR` | `OFF` | ROOTの2次元・3次元ベクトル型 |
| `ANLNEXT_USE_HEPVECTOR` | `OFF` | CLHEPのベクトル型 |
| `ANLNEXT_USE_G4VECTOR` | `OFF` | Geant4付属CLHEPのベクトル型 |
| `BUILD_TESTING` | `ON` | 回帰テストとテスト用バインディング |

外部ライブラリに依存する型を使う場合は、本体と利用側でビルド条件を揃えてください。

## インタプリタから読み込む

```sh
export RUBYLIB="$ANLNEXT_INSTALL/lib/ruby${RUBYLIB:+:$RUBYLIB}"
export PYTHONPATH="$ANLNEXT_INSTALL/lib/python${PYTHONPATH:+:$PYTHONPATH}"
ruby -e 'require "anlnext"; puts ANL.status_to_string(ANL::AS_OK)'
python3 -c 'import anlnext; print(anlnext.status_to_string(anlnext.status.AS_OK))'
```

両方で`AS_PROCEED`と表示されれば、バインディングを読み込めています。
`AS_OK`は`AS_PROCEED`の正式な別名です。
Pythonは、ビルド時に選んだものと対応するインタプリタで実行してください。

## Submodule例題のビルド

リポジトリのルートから次を実行します。

```sh
cmake -S examples/submodule/source -B build-submodule \
  -DANLNEXT_INSTALL="$ANLNEXT_INSTALL" \
  -DCMAKE_INSTALL_PREFIX="$ANLNEXT_INSTALL"
cmake --build build-submodule --parallel 4
cmake --install build-submodule
```

この例題は、親モジュール`AlgorithmModule`と、乗算・加算を行うSubmoduleを提供します。
次の二つのスクリプトは同じ解析です。どちらか一方を保存して実行できます。

### Ruby

```ruby
require 'anlnext'
require 'submoduleExample'

chain = ANL::AnalysisChain.new
chain.console = false
chain.num_parallels = 3
chain.chain(SubmoduleExample::AlgorithmModule, :processor)
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters(input: 4.0, algorithm_: {factor: 7.0})
chain.run(10)

mod = chain.get_module(:processor)
puts mod.get_parameter_value('total')
```

### Python

```python
import anlnext
from submoduleExamplePy import AlgorithmModule

chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = 3
chain.chain(AlgorithmModule, 'processor')
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters({'input': 4.0, 'algorithm_': {'factor': 7.0}})
chain.run(10)

module = chain.get_module('processor')
print(anlnext.get_value(module.get_parameter('total')))
```

1反復で`4 × 7 = 28`を計算し、10反復の合計として`280.0`を表示します。
`num_parallels = 3`は、各チェーンが10回ずつ実行する指定ではありません。
全チェーンで合計10回を分担します。`1`にすると単一チェーンで実行します。

同梱の実行スクリプトでは、乗算・加算の両方を試せます。

```sh
ruby examples/submodule/run/run_submodule.rb 3
python3 examples/submodule/run_py/run_submodule.py 3
```

## テストを実行する

CTestにはRuby／PythonおよびYAMLのテストも含まれます。
PyYAMLを利用できるPythonを指定して、本体の設定を変更します。
必要なら仮想環境を作り、そのPythonにPyYAMLをインストールしてください。

```sh
cmake -S . -B build-v3 -DBUILD_TESTING=ON \
  -DPython3_EXECUTABLE=/path/to/python-with-pyyaml
cmake --build build-v3 --parallel 4
ctest --test-dir build-v3 --output-on-failure --parallel 4
```

テスト用ビルドは、インストール済みのANLNextを上書きせずにバインディングを検証します。
