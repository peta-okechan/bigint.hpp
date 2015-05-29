#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <bitset>
#include <string>

/*
 * 多倍長整数クラス
 * バカ正直実装版
 */

struct BigInt
{
private:
  // 数値の絶対値の情報（32bitずつ下位桁から順に保持）
  std::vector<uint32_t> digits;
  // 符号（0以上はfalse）
  bool minus;
  // NaNかどうか
  bool nan;

  // 64bitのデータを下位32bit上位32bitに分割する（という場面で利用）
  template<typename T>
  static std::tuple<uint32_t, uint32_t> split(const T &v)
  {
    return std::tuple<uint32_t, uint32_t>(low(v), high(v));
  }

  // 64bitのデータの下位32bitを返す
  template<typename T>
  static const uint32_t low(const T &v)
  {
    return reinterpret_cast<const uint32_t*>(&v)[0];
  }

  // 64bitのデータの上位32bitを返す
  template<typename T>
  static const uint32_t high(const T &v)
  {
    return reinterpret_cast<const uint32_t*>(&v)[1];
  }

public:
  // ゼロを表現するよう初期化
  BigInt()
    :digits(1, 0), minus(false), nan(false)
  {}

  // uint32_tからの変換
  BigInt(uint32_t value)
    :digits(1, value), minus(false), nan(false)
  {}

  // uint64_tからの変換
  BigInt(uint64_t value)
    :digits(2), minus(false), nan(false)
  {
    std::tie(digits[0], digits[1]) = split(value);
    normalize();
  }

  // int32_tからの変換
  BigInt(int32_t value)
    :digits(1, std::abs(value)), minus(value < 0), nan(false)
  {}

  // int64_tからの変換
  BigInt(int64_t value)
    :digits(2), minus(value < 0), nan(false)
  {
    std::tie(digits[0], digits[1]) = split(std::abs(value));
    normalize();
  }

  // 文字列からの変換（数字以外は先頭に符号を表記可 カンマは無視するので区切り文字として使用可）
  BigInt(const std::string value)
    :digits(1, 0), minus(false), nan(false)
  {
    for (size_t i = 0; i < value.size(); ++i)
    {
      auto v = value.substr(i, 1);
      if (i == 0 && (v == "-" || v == "+")) continue;
      if (i != 0 && v == ",") continue;
      *this *= BigInt(10);
      *this += BigInt(std::stoul(value.substr(i, 1)));
    }

    minus = (value.size() > 0 && value.substr(0, 1) == "-");
    normalize();
  }

  virtual ~BigInt() = default;

  // ゼロにする
  void zeroize()
  {
    digits.clear();
    digits.push_back(0);
    minus = false;
    nan = false;
  }

  // 正規化（上位桁の無駄なゼロを削除 最低1桁は持つようにする）
  void normalize()
  {
    if (nan)
    {
      *this = NaN();
      return;
    }

    for (size_t i = digits.size(); i > 0; --i)
    {
      if (digits[i-1] != 0) break;
      digits.pop_back();
    }
    if (digits.size() == 0) zeroize();
  }

  // 正規化（破壊的変更を行わない版）
  BigInt normalized() const
  {
    BigInt result = *this;
    result.normalize();
    return result;
  }

  // 符号反転
  BigInt &sign_inv()
  {
    minus = !minus;
    return *this;
  }

  // 絶対値にする
  void abs()
  {
    minus = false;
  }

  // 絶対値を返す
  BigInt abs() const
  {
    auto result = BigInt(*this);
    result.abs();
    return result;
  }

  // マイナスかどうか
  bool isMinus() const
  {
    return minus;
  }

  // NaNかどうか
  bool isNaN() const
  {
    return nan;
  }

  // 桁数（2＾32進数の桁数）
  size_t size() const
  {
    return digits.size();
  }

  // 各桁のデータを返す
  uint32_t operator[](const size_t index) const
  {
    return digits[index];
  }

  // マイナス符号（単項演算子）
  BigInt operator-() const
  {
    BigInt result = *this;
    result.sign_inv();
    result.nan = nan;
    return result;
  }

  // 等しいかどうか（符号も桁のデータも全て等しい時のみ等しい）
  bool operator==(const BigInt &rhs) const
  {
    if (nan || rhs.nan) return false;

    auto L = normalized();
    auto R = rhs.normalized();

    if (L.minus != R.minus) return false;
    if (L.size() != R.size()) return false;
    for (size_t i = 0; i < L.size(); ++i)
    {
      if (L[i] != R[i]) return false;
    }
    return true;
  }

  // 等しくないかどうか（イコールの否定）
  bool operator!=(const BigInt &rhs) const
  {
    if (nan || rhs.nan) return false;

    return !(*this == rhs);
  }

  // 大なり
  bool operator>(const BigInt &rhs) const
  {
    if (nan || rhs.nan) return false;

    auto L = normalized();
    auto R = rhs.normalized();

    if (L.minus != R.minus) return !minus;

    if (L.minus) std::swap(L, R);

    if (L.size() != R.size())
      return L.size() > R.size();

    for (size_t i = L.size(); i > 0; --i)
    {
      if (L[i-1] != R[i-1]) return L[i-1] > R[i-1];
    }

    return false;
  }

  // 大なりイコール（>と==のor）
  bool operator>=(const BigInt &rhs) const
  {
    if (nan || rhs.nan) return false;

    return ((*this == rhs) || (*this > rhs));
  }

  // 小なり（大なりイコールの否定）
  bool operator<(const BigInt &rhs) const
  {
    if (nan || rhs.nan) return false;

    return !(*this >= rhs);
  }

  // 小なりイコール（大なりの否定）
  bool operator<=(const BigInt &rhs) const
  {
    if (nan || rhs.nan) return false;

    return !(*this > rhs);
  }

  // 加算（符号が同じ場合は絶対値の加算 符号が違う場合は絶対値の減算 符号は適当に）
  BigInt operator+(const BigInt &rhs) const
  {
    if (minus == rhs.minus)
    {
      auto result = abs_add(*this, rhs);
      result.minus = minus;
      result.nan = (nan || rhs.nan);
      return result.normalized();
    }
    else
    {
      auto result = abs_sub(*this, rhs);
      result.minus = (abs() >= rhs.abs())?minus:rhs.minus;
      result.nan = (nan || rhs.nan);
      return result.normalized();
    }
  }

  // 加算して代入
  BigInt &operator+=(const BigInt &rhs)
  {
    *this = *this + rhs;
    return *this;
  }

  // 減算（rhsを符号反転して加算に変形）
  BigInt operator-(const BigInt &rhs) const
  {
    return *this + -rhs;
  }

  // 減算して代入
  BigInt &operator-=(const BigInt &rhs)
  {
    *this = *this - rhs;
    return *this;
  }

  // 乗算（絶対値で乗算 符号は適当に）
  BigInt operator*(const BigInt &rhs) const
  {
    auto result = abs_mul(*this, rhs);
    result.minus = (minus != rhs.minus);
    result.nan = (nan || rhs.nan);
    return result.normalized();
  }

  // 乗算して代入
  BigInt &operator*=(const BigInt &rhs)
  {
    *this = *this * rhs;
    return *this;
  }

  // 除算（絶対値で除算 符号は適当に）
  BigInt operator/(const BigInt &rhs) const
  {
    auto result = abs_div(*this, rhs);
    result.minus = (minus != rhs.minus);
    result.nan = (result.nan || nan || rhs.nan);

    // Pythonに合わせて、どちらかの引数が負かつ余りが出る場合、負の無限大方向にまるめる
    if (minus != rhs.minus && *this != rhs * result)
    {
      result -= BigInt(1);
    }

    return result.normalized();
  }

  // 除算して代入
  BigInt &operator/=(const BigInt &rhs)
  {
    *this = *this / rhs;
    return *this;
  }

  // 剰余（除算の結果から計算 符号の関係性はPythonを参考）
  BigInt operator%(const BigInt &rhs) const
  {
    auto &L = *this;
    auto &R = rhs;
    auto result = L - (L / R) * R;
    return result;
  }

  // 剰余を代入
  BigInt &operator%=(const BigInt &rhs)
  {
    *this = *this % rhs;
    return *this;
  }

  // NaN
  static BigInt NaN()
  {
    BigInt result;
    result.nan = true;
    return result;
  }

  // 絶対値どうしの加算（呼び出し側は絶対値を渡す必要はない）
  static BigInt abs_add(const BigInt &lhs, const BigInt &rhs)
  {
    if (lhs.nan || rhs.nan) return NaN();

    auto L = lhs.abs();
    auto R = rhs.abs();
    BigInt result;
    result.digits.clear();
    bool carry = false;
    size_t size = std::max(L.size(), R.size());

    for (size_t i = 0; i < size; ++i)
    {
      uint64_t ld = (i < L.size())?L[i]:0;
      uint64_t rd = (i < R.size())?R[i]:0;

      uint64_t st = ld + rd + (carry?1:0);

      result.digits.push_back(low(st));
      carry = (high(st));
    }
    if (carry)
    {
      result.digits.push_back(1);
    }

    result.normalize();
    return result;
  }

  // 絶対値どうしの減算（呼出側は絶対値を渡す必要はない 自動的に絶対値が大きいほうから減算する）
  static BigInt abs_sub(const BigInt &lhs, const BigInt &rhs)
  {
    if (lhs.nan || rhs.nan) return NaN();

    auto L = lhs.abs();
    auto R = rhs.abs();
    BigInt result;
    result.digits.clear();
    bool carry = false;
    size_t size = std::max(L.size(), R.size());

    if (L < R) std::swap(L, R);

    for (size_t i = 0; i < size; ++i)
    {
      uint64_t ld = (i < L.size())?L[i]:0;
      uint64_t rd = (i < R.size())?R[i]:0;

      uint64_t st;
      if (ld - (carry?1:0) >= rd)
      {
        st = ld - rd - (carry?1:0);
        carry = false;
      }
      else
      {
        st = ld + 0x100000000 - rd - (carry?1:0);
        carry = true;
      }
      result.digits.push_back(low(st));
    }
    if (carry)
    {
      throw "BigInt::abs_sub : carry error!";
    }

    result.normalize();
    return result;
  }

  // 絶対値どうしの乗算（呼出側は絶対値を渡す必要はない 筆算と同じアルゴリズム）
  static BigInt abs_mul(const BigInt &lhs, const BigInt &rhs)
  {
    if (lhs.nan || rhs.nan) return NaN();

    auto L = lhs.abs();
    auto R = rhs.abs();
    BigInt result;
    result.digits.clear();
    BigInt tmp;
    tmp.digits.resize(L.size() + R.size());

    for (size_t i = 0; i < R.size(); ++i)
    {
      uint64_t carry = 0;
      uint64_t rd = R[i];
      if (i > 0) tmp.digits[i-1] = 0;

      for (size_t j = 0; j < L.size(); ++j)
      {
        uint64_t ld = L[j];
        uint64_t st = ld * rd + carry;
        tmp.digits[i + j] = low(st);
        carry = high(st);
      }

      if (carry) tmp.digits[i + L.size()] = carry;
      result += tmp;
    }

    result.normalize();
    return result;
  }

  // 絶対値どうしの除算（呼出側は絶対値を渡す必要はない 筆算と同じアルゴリズム）
  static BigInt abs_div(const BigInt &lhs, const BigInt &rhs)
  {
    if (lhs.nan || rhs.nan) return NaN();
    if (rhs.size() == 1 && rhs[0] == 0)
    {
      BigInt result;
      result.nan = true;
      //std::cerr << "Divide by zero!" << std::endl;
      return result;
    }

    auto L = lhs.abs();
    auto R = rhs.abs();
    if (L < R) return BigInt(0);
    if (L == R) return BigInt(1);

    BigInt result;
    result.digits.clear();
    BigInt tmp;
    tmp.digits.clear();
    size_t i = L.size() - R.size();
    for (size_t j = i + 1; j < L.size(); ++j)
    {
      tmp.digits.push_back(L[j]);
    }

    for (; i + 1 > 0; --i)
    {
      uint32_t st = 0;  // 答えの一桁分（筆算で上位から一桁ずつ答えを出していくアレ）
      tmp *= BigInt(0x100000000);
      tmp += BigInt(L[i]);

      // 一桁分の商を探索する（32bit空間を二分探索）
      if (tmp == R) st = 1;
      else if (tmp < R) st = 0;
      else
      {
        uint32_t range = 0x100000000 / 2;
        uint32_t s = range;
        for (int j = 0; j < 32; ++j)
        {
          // Rと仮の商（の一桁）sを掛けてtmp以下で一番大きい数かどうか判定
          auto p = R * BigInt(s);
          if (tmp >= p && (tmp - p) < R)
          {
            // 商の一桁分が確定
            st = s;
            break;
          }
          range /= 2;
          s += (tmp > p)?range:-range;
        }
      }

      result.digits.push_back(st);
      tmp -= R * BigInt(st);
    }

    // resultには商の各桁が逆順に入ってる
    std::reverse(result.digits.begin(), result.digits.end());
    result.normalize();

    return result;
  }

  void print_bits(bool hex = true) const
  {
    auto flags = std::cout.flags();
    std::cout << (minus?"-":"+") << (hex?"0x":"0b");
    if (digits.size() == 0)
    {
      std::cout << "0";
    }
    else
    {
      for (size_t i = digits.size() - 1;; --i)
      {
        if (hex)
          std::cout << std::setfill('0') << std::setw(8) << std::hex << digits[i];
        else
          std::cout << static_cast<std::bitset<32>>(digits[i]);
        if (i == 0) break;
      }
    }
    std::cout << std::endl;
    std::cout.flags(flags);
  }

  std::string to_string() const
  {
    if (nan) return "NaN";

    auto tmp = abs();
    std::string result;
    std::ostringstream oss;

    while (tmp > BigInt(0))
    {
      oss.str("");
      oss.clear(std::stringstream::goodbit);
      oss << std::setw(9) << std::setfill('0');

      auto s = tmp / BigInt(1000000000);
      auto r = tmp - s * BigInt(1000000000);
      oss << r[0];
      result = oss.str() + result;
      tmp = s;
    }

    if (result.size() == 0) result = "0";

    // 先頭のゼロを削除する
    size_t i;
    for (i = 0; result.substr(i, 1) == "0" && i < result.size() - 1; ++i);
    result = result.substr(i);

    if (minus) result = "-" + result;
    return result;
  }
};
