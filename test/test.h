// Type declarations
//struct {int f : 4;}
//enum { FOO = 3 * 4, BAR, LEX }
//enum foo { FOO = 0, BAR, LEX }
//struct foo{int x; struct bar{float f; short b;} a;}
//union foo{int x; int y;}
//struct foo{int x; int y;}
//int (*)(int foo)
//int (*(*)(int (*)(int, int), int))(int, int)
//int(*)()[2]
//int[512][321]
//int(*)(const static int unsigned, int, ...)
//int(*)(const static int unsigned)
//int[]
//char**
//int(*)(int)
//const int* volatile
//int**
//int*


// Expressions
sizeof(struct{int f; char p;})
//(int*)b + c
//a + ()b
//a > b ? 1 : 2
//x + 'a'
//a << 2 >> 4
//a < b >= c
//a == b && a != b
//1 & true & false
//1 ^ true ^ false | hello
//1 | true | false
//1 && false && true
//1 || false || foo
//foo *= 3 += 4
//3*(2+1)*"asuduha"
//foo(--i, &a, *b, ++c, -d, +e, ~f, !g)
//foo[32][43](123, x->x, i++, y.hello[123].x, i--)
