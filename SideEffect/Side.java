public class Side {
    public static int x;
    public static int foo () {
	x = 13;
	return 42;
    }
    public static int bar () {
	x = 14;
	return 42;
    }
    public static int fun (int a, int b) {
	return a + b;
    }
    public static void main (String[] args) {
	fun(foo(), bar());
	System.out.println("x = " + x);
    }
}
