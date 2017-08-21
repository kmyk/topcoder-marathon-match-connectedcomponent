#pragma GCC optimize "O3"
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <memory>
#include <array>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <cassert>
#define repeat(i, n) for (int i = 0; (i) < int(n); ++(i))
#define repeat_from(i, m, n) for (int i = (m); (i) < int(n); ++(i))
#define repeat_reverse(i, n) for (int i = (n)-1; (i) >= 0; --(i))
#define repeat_from_reverse(i, m, n) for (int i = (n)-1; (i) >= int(m); --(i))
#define whole(x) begin(x), end(x)
using namespace std;
const int dy[] = { -1, 1, 0, 0 };
const int dx[] = { 0, 0, 1, -1 };
bool is_on_field(int y, int x, int h, int w) { return 0 <= y and y < h and 0 <= x and x < w; }
template <class T> inline void setmax(T & a, T const & b) { a = max(a, b); }
template <class T> inline void setmin(T & a, T const & b) { a = min(a, b); }

double rdtsc() { // in seconds
    constexpr double ticks_per_sec = 2500000000;
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32 | lo) / ticks_per_sec;
}

constexpr double eps = 1e-6;
default_random_engine gen;

class ConnectedComponent { public: vector<int> permute(vector<int> matrix); };
constexpr int MAX_S = 500;

double calculate_score(vector<int> const & p, vector<char> const & matrix) {
    int s = p.size();
    auto at = [&](int y, int x) { return matrix[p[y] * s + p[x]]; };
    auto is_on_field = [&](int y, int x) { return 0 <= y and y < s and 0 <= x and x < s; };
    vector<char> used(s * s);
    int size = 0, acc = 0;
    function<void (int, int)> go = [&](int y, int x) {
        used[y * s + x] = true;
        size += 1;
        acc += at(y, x);
        repeat (i, 4) {
            int ny = y + dy[i];
            int nx = x + dx[i];
            if (is_on_field(ny, nx) and not used[ny * s + nx] and at(ny, nx)) {
                go(ny, nx);
            }
        }
    };
    double result = 0;
    repeat (y, s) repeat (x, s) {
        if (not used[y * s + x] and at(y, x)) {
            size = acc = 0;
            go(y, x);
            setmax(result, acc * sqrt(size));
        }
    }
    return result;
}

struct rectangle_t {
    int ly, lx, ry, rx;
};
bool is_contained(rectangle_t a, rectangle_t b) {
    return b.ly <= a.ly and a.ry <= b.ry
        and b.lx <= a.lx and a.rx <= b.rx;
}
bool is_contained(int y, int x, rectangle_t b) {
    return b.ly <= y and y < b.ry
        and b.lx <= x and x < b.rx;
}

pair<int, int> find_the_center(vector<int> const & p, vector<char> const & matrix) {
    int s = p.size();
    auto at = [&](int y, int x) { return matrix[p[y] * s + p[x]]; };
    for (int delta = 0; delta < s; ++ delta) {
        int l = s / 2 - delta;
        int r = s / 2 + delta + 1;
        repeat_from (y, l, r) {
            int d = (abs(y - s / 2) == delta ? 1 : r - l - 1);
            for (int x = l; x < r; x += d) {
                if (at(y, x) >= 1) {
                    return { y, x };
                }
            }
        }
    }
    assert (false);
}

char g_used[MAX_S * MAX_S] = {};
int16_t g_stack[2 * MAX_S * MAX_S];
int g_stack_size = 0;

struct prepared_slice_t {
    rectangle_t rect;
    int size, sum;
    vector<tuple<int16_t, int16_t, int16_t, int16_t> > frontier;
};
struct prepared_info_t {
    int cy, cx;
    rectangle_t rect;
    int size, sum;
    vector<shared_ptr<prepared_slice_t> > slices;
};
prepared_info_t prepare_to_analyze(vector<int> const & p, vector<char> const & matrix) {
    int s = p.size();
    auto at = [&](int y, int x) { return matrix[p[y] * s + p[x]]; };
    auto is_on_field = [&](int y, int x) { return 0 <= y and y < s and 0 <= x and x < s; };
    prepared_info_t info = {};
    tie(info.cy, info.cx) = find_the_center(p, matrix);
    info.rect.ly = s;
    info.rect.lx = s;
    info.rect.ry = 0;
    info.rect.rx = 0;
    info.size = 0;
    info.sum = 0;
    int radius = 10;
    auto push = [&](int y, int x) {
#ifdef LOCAL
assert (not g_used[y * s + x]);
assert (at(y, x));
#endif
        g_used[y * s + x] = true;
        g_stack[g_stack_size ++] = y;
        g_stack[g_stack_size ++] = x;
        setmin(info.rect.ly, y);
        setmax(info.rect.ry, y + 1);
        setmin(info.rect.lx, x);
        setmax(info.rect.rx, x + 1);
        info.size += 1;
        info.sum += at(y, x);
    };
    push(info.cy, info.cx);
    while (true) {
        shared_ptr<prepared_slice_t> slice = make_shared<prepared_slice_t>();
        while (g_stack_size) {
            int x = g_stack[-- g_stack_size];
            int y = g_stack[-- g_stack_size];
            repeat (i, 4) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                if (is_on_field(ny, nx)) {
                    if (info.cy - radius <= ny and ny <= info.cy + radius
                            and info.cx - radius <= nx and nx <= info.cx + radius) {
                        if (not g_used[ny * s + nx] and at(ny, nx)) {
                            push(ny, nx);
                        }
                    } else {
                        slice->frontier.emplace_back(ny, nx, y, x);
                    }
                }
            }
        }
        slice->rect = info.rect;
        slice->sum  = info.sum;
        slice->size = info.size;
        for (auto it : slice->frontier) {
            int y, x; tie(y, x, ignore, ignore) = it;
            if (not g_used[y * s + x] and at(y, x)) {
                push(y, x);
            }
        }
        info.slices.push_back(slice);
        if (slice->frontier.empty()) break;
        radius += 10;
    }
    repeat_from (y, info.rect.ly, info.rect.ry) {
        memset((void *)(g_used + y * s + info.rect.lx), 0, (size_t)(info.rect.rx - info.rect.lx));
    }
    return info;
}

struct permutation_info_t {
    double score;
    double evaluated;
    int cy, cx;
    rectangle_t rect;
    int size, sum;
};
permutation_info_t analyze_permutation(vector<int> const & p, vector<char> const & matrix, prepared_info_t const & prepared, rectangle_t preserved) {
    int s = p.size();
    auto at = [&](int y, int x) { return matrix[p[y] * s + p[x]]; };
    auto is_on_field = [&](int y, int x) { return 0 <= y and y < s and 0 <= x and x < s; };
    permutation_info_t info = {};
    auto push = [&](int y, int x) {
#ifdef LOCAL
assert (not g_used[y * s + x]);
assert (at(y, x));
#endif
        g_used[y * s + x] = true;
        g_stack[g_stack_size ++] = y;
        g_stack[g_stack_size ++] = x;
        setmin(info.rect.ly, y);
        setmax(info.rect.ry, y + 1);
        setmin(info.rect.lx, x);
        setmax(info.rect.rx, x + 1);
        info.size += 1;
        info.sum += at(y, x);
    };
    if (not is_contained(prepared.cy, prepared.cx, preserved)) {
        int cy, cx; tie(cy, cx) = find_the_center(p, matrix);
        push(cy, cx);
    } else {
        info.cy = prepared.cy;
        info.cx = prepared.cx;
        if (not is_contained(prepared.slices[0]->rect, preserved)) {
            push(info.cy, info.cx);
            info.rect.ly = info.cy;
            info.rect.lx = info.cx;
            info.rect.ry = info.cy + 1;
            info.rect.rx = info.cx + 1;
        } else {
            auto it = prepared.slices.begin();
            while (next(it) != prepared.slices.end() and is_contained((*next(it))->rect, preserved)) ++ it;
            auto & slice = **it;
            info.rect = slice.rect;
            info.sum  += slice.sum;
            info.size += slice.size;
            for (auto frontier_i : slice.frontier) {
                int y, x, py, px; tie(y, x, py, px) = frontier_i;
                if (at(y, x)) {
                    g_used[py * s + px] = true;
                    push(y, x);
                }
            }
        }
    }
    while (g_stack_size) {
        int x = g_stack[-- g_stack_size];
        int y = g_stack[-- g_stack_size];
        repeat (i, 4) {
            int ny = y + dy[i];
            int nx = x + dx[i];
            if (is_on_field(ny, nx) and not g_used[ny * s + nx] and at(ny, nx)) {
                push(ny, nx);
            }
        }
    }
    repeat_from (y, info.rect.ly, info.rect.ry) {
        memset((void *)(g_used + y * s + info.rect.lx), 0, (size_t)(info.rect.rx - info.rect.lx));
    }
    info.score = info.sum * sqrt(info.size);
    info.evaluated = info.score;
    return info;
}

#ifdef VISUALIZE
void visualize(vector<int> const & p) {
    cerr << "VISUALIZE: " << p.size();
    for (int p_i : p) cerr << ' ' << p_i;
    cerr << endl;
}
#endif

double estimate_base_score(int s, vector<char> const & matrix) {
    vector<int> p(s);
    iota(whole(p), 0);
    deque<double> scores;
    repeat (i, 16) {
        shuffle(whole(p), gen);
        scores.push_back(calculate_score(p, matrix));
    }
    sort(whole(scores));
    scores.pop_front();
    scores.pop_front();
    scores.pop_front();
    scores.pop_back();
    scores.pop_back();
    scores.pop_back();
    return accumulate(whole(scores), 0.0) / 10;
}

vector<int> ConnectedComponent::permute(vector<int> int_matrix) {
    // prepare
    double clock_begin = rdtsc();
    vector<char> matrix(whole(int_matrix));
    int s = int(sqrt(matrix.size()));
    // initialize
    vector<int> p(s);
    iota(whole(p), 0);
#ifdef VISUALIZE
visualize(p);
#endif
#ifdef LOCAL
cerr << "MESSAGE: s = " << s << endl;
#endif
    {
        vector<int> cnt(s);
        repeat (y, s) {
            repeat (x, s) {
                int m = matrix[p[y] * s + p[x]];
                int value = m > 0 ? m + 4 : m == 0 ? -4 : m;
                cnt[p[y]] += value;
                cnt[p[x]] += value;
            }
        }
        vector<int> t(s);
        iota(whole(t), 0);
        sort(whole(t), [&](int i, int j) { return cnt[i] > cnt[j]; });
        p.clear();
        for (int i = 0; i < s; i += 2) p.push_back(t[i]);
        reverse(whole(p));
        for (int i = 1; i < s; i += 2) p.push_back(t[i]);
#ifdef VISUALIZE
visualize(p);
#endif
    }
    prepared_info_t prepared = prepare_to_analyze(p, matrix);
    permutation_info_t current = analyze_permutation(p, matrix, prepared, (rectangle_t) { 0, 0, s, s });
    vector<int> result = p;
    double best_score = current.score;
    // simulated annealing
    double temp = INFINITY;
    double time = 0.0;
    for (int iteration = 0; ; ++ iteration) {
        if (iteration % 10 == 0 or time > 0.95) {
            double clock_end = rdtsc();
            time = (clock_end - clock_begin) / 10.0;
            if (time > 0.98) {
#ifdef LOCAL
cerr << "MESSAGE: iteration = " << iteration << endl;
cerr << "MESSAGE: ratio = " << best_score / estimate_base_score(s, matrix) << endl;
cerr << "MESSAGE: time = " << time << endl;
cerr << "MESSAGE: avg m = " << accumulate(whole(matrix), 0) /(double) (s * s) << endl;
int cnt[3] = {};
repeat (z, s * s) cnt[matrix[z] > 0 ? 0 : matrix[z] == 0 ? 1 : 2] += 1;
cerr << "MESSAGE: positive : zero : negative = "
    << cnt[0] /(double) (s * s) << " : "
    << cnt[1] /(double) (s * s) << " : "
    << cnt[2] /(double) (s * s) << endl;
#endif
                break;
            }
            temp = (1 - time) * s;
        }
        constexpr int neightborhood_type_swap = 4;
        constexpr int neightborhood_type_rotate = 4;
        constexpr int neightborhood_type_reverse = 2;
        int neightborhood_type = uniform_int_distribution<int>(0,
                + neightborhood_type_swap
                + neightborhood_type_rotate
                + neightborhood_type_reverse
                - 1)(gen);
        int x = -1, y = -1;
        while (x == y) {
            x = bernoulli_distribution(0.5)(gen) ?
                uniform_int_distribution<int>(max(0, prepared.rect.ly - 3), min(s, prepared.rect.ry + 3) - 1)(gen) :
                uniform_int_distribution<int>(max(0, prepared.rect.lx - 3), min(s, prepared.rect.rx + 3) - 1)(gen);
            y = uniform_int_distribution<int>(0, s - 1)(gen);
        }
        rectangle_t preserved = { -1, -1, -1, -1 };
        if (neightborhood_type < neightborhood_type_swap) {
            int l = min(x, y) + 1;
            int r = max(x, y);
            rectangle_t rect = { l, l, r, r };
            if (is_contained(prepared.cy, prepared.cx, rect)) {
                preserved = rect;
            }
        } else {
            int l = min(x, y);
            int r = max(x, y) + 1;
            if (prepared.cy < l and prepared.cx < l) {
                preserved = { 0, 0, l, l };
            } else if (r <= prepared.cy and r <= prepared.cx) {
                preserved = { r, r, s, s };
            }
        }
        if (neightborhood_type < neightborhood_type_swap) {
            swap(p[x], p[y]);
        } else if (neightborhood_type < neightborhood_type_swap + neightborhood_type_rotate) {
            if (x < y) {
                rotate(p.begin() + x, p.begin() + (x + 1), p.begin() + (y + 1));
            } else {
                rotate(p.begin() + y, p.begin() + x, p.begin() + (x + 1));
            }
        } else {
            reverse(p.begin() + min(x, y), p.begin() + (max(x, y) + 1));
        }
        auto next = analyze_permutation(p, matrix, prepared, preserved);
        double delta = next.evaluated - current.evaluated;
        if (current.evaluated < next.evaluated + 10 or bernoulli_distribution(exp(delta / temp))(gen)) {
            current.evaluated = next.evaluated;
            prepared = prepare_to_analyze(p, matrix);
            if (best_score < next.score) {
                best_score = next.score;
                current = next;
                result = p;
#ifdef VISUALIZE
visualize(p);
#endif
#ifdef LOCAL
cerr << "MESSAGE: iteration = " << iteration << endl;
cerr << "MESSAGE: evaluated = " << int(current.evaluated) << endl;
cerr << "MESSAGE: time = " << time << endl;
cerr << "MESSAGE: score = " << int(best_score) << endl;
cerr << "MESSAGE: size = " << current.size << endl;
cerr << "MESSAGE: sum = " << current.sum  << endl;
#endif
            }
        } else {
            if (neightborhood_type < neightborhood_type_swap) {
                swap(p[x], p[y]);
            } else if (neightborhood_type < neightborhood_type_swap + neightborhood_type_rotate) {
                if (x < y) {
                    rotate(p.begin() + x, p.begin() + y, p.begin() + (y + 1));
                } else {
                    rotate(p.begin() + y, p.begin() + (y + 1), p.begin() + (x + 1));
                }
            } else {
                reverse(p.begin() + min(x, y), p.begin() + (max(x, y) + 1));
            }
        }
    }
#ifdef VISUALIZE
cerr << "MESSAGE: score = " << best_score << endl;
#endif
    return result;
}

// -------8<------- end of solution submitted to the website -------8<-------

template<class T> void getVector(vector<T>& v) {
    for (int i = 0; i < (int)v.size(); ++i)
        cin >> v[i];
}

int main() {
    ConnectedComponent cc;
    int M;
    cin >> M;
    vector<int> matrix(M);
    getVector(matrix);

    vector<int> ret = cc.permute(matrix);
    cout << ret.size() << endl;
    for (int i = 0; i < (int)ret.size(); ++i)
        cout << ret[i] << endl;
    cout.flush();
}
