public class TestBitwiseOperations {
    public static void main(String[] args) {
        int a = 5;  // 0101 in binary
        int b = 3;  // 0011 in binary

        check("a & b", a & b, 1);
        check("a | b", a | b, 7);
        check("a ^ b", a ^ b, 6);
        check("~a", ~a, -6);
        check("a << 1", a << 1, 10);
        check("a >> 1", a >> 1, 2);

        // Test with long values
        long la = 5L;  // 0101 in binary
        long lb = 3L;  // 0011 in binary

        check("la & lb", la & lb, 1L);
        check("la | lb", la | lb, 7L);
        check("la ^ lb", la ^ lb, 6L);
        check("~la", ~la, -6L);
        check("la << 1", la << 1, 10L);
        check("la >> 1", la >> 1, 2L);

        // Test with byte values
        byte ba = 5;  // 0101 in binary
        byte bb = 3;  // 0011 in binary
        check("ba & bb", (byte)(ba & bb), (byte)1);
        check("ba | bb", (byte)(ba | bb), (byte)7);
        check("ba ^ bb", (byte)(ba ^ bb), (byte)6);
        check("~ba", (byte)(~ba), (byte)-6);
        check("ba << 1", (byte)(ba << 1), (byte)10);
        check("ba >> 1", (byte)(ba >> 1), (byte)2);

        // Test with short values
        short sa = 5;  // 0101 in binary
        short sb = 3;  // 0011 in binary
        check("sa & sb", (short)(sa & sb), (short)1);
        check("sa | sb", (short)(sa | sb), (short)7);
        check("sa ^ sb", (short)(sa ^ sb), (short)6);
        check("~sa", (short)(~sa), (short)-6);
        check("sa << 1", (short)(sa << 1), (short)10);
        check("sa >> 1", (short)(sa >> 1), (short)2);

        // Test with char values
        char ca = 5;  // 0101 in binary
        char cb = 3;  // 0011 in binary
        check("ca & cb", (char)(ca & cb), (char)1);
        check("ca | cb", (char)(ca | cb), (char)7);
        check("ca ^ cb", (char)(ca ^ cb), (char)6);
        check("~ca", (char)(~ca), (char)0xFFFA); // ~5 = 0xFFFFFFFA, char is 16-bit unsigned
        check("ca << 1", (char)(ca << 1), (char)10);
        check("ca >> 1", (char)(ca >> 1), (char)2);
    }

    private static void check(String label, int actual, int expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, long actual, long expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, byte actual, byte expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, short actual, short expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, char actual, char expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + (int)expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + (int)actual + " (expected " + (int)expected + ")" + ConsoleColors.RESET);
        }
    }
}
