class pair
{
  int x, y;
public:
  constexpr pair() : x(), y() { }
  constexpr pair(int x, int y) : x(x), y(y) { }
  constexpr int get_x() const { return x; }
  friend constexpr bool operator<(const pair & lhs, const pair & rhs);
  friend std::ostream & operator<<(std::ostream & os, const pair & rhs);
};
