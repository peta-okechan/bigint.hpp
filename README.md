# bigint.hpp
C++の多倍長整数クラス(1ソース)

多倍長整数をバカ正直に実装（乗算や除算を筆算と同じアルゴリズムで実装）したものです。
実装は全てbigint.hppに入っています。

まず、
python bitestgen.py > tests.txt
でテストケースを生成した後、
clang++ -std=c++14 -O3 bitest.cpp -o bitest && ./bitest
でテストできます。

Fedora 22(x86_64)のパッケージで入るclang 3.5.0で確認してあります。
