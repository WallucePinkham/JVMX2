public class TestArithmetic {
    public static void main(String[] args) {
        // int arithmetic
        int a = 5, b = 3;
        check("int: a + b", a + b, 8);
        check("int: a - b", a - b, 2);
        check("int: a * b", a * b, 15);
        check("int: a / b", a / b, 1);
        check("int: a % b", a % b, 2);

        // long arithmetic
        long la = 5L, lb = 3L;
        check("long: la + lb", la + lb, 8L);
        check("long: la - lb", la - lb, 2L);
        check("long: la * lb", la * lb, 15L);
        check("long: la / lb", la / lb, 1L);
        check("long: la % lb", la % lb, 2L);

        // float arithmetic
        float fa = 3.14159f, fb = 2.71828f;
        check("float: fa + fb", fa + fb, 5.85987f);
        check("float: fa - fb", fa - fb, 0.42330998f);
        check("float: fa * fb", fa * fb, 8.539721f);
        check("float: fa / fb", fa / fb, 1.1557276f);
        check("float: fa % fb", fa % fb, 0.42330998f);

        // double arithmetic
        double da = 3.14159, db = 2.71828;
        check("double: da + db", da + db, 5.85987);
        check("double: da - db", da - db, 0.42330999999999985);
        check("double: da * db", da * db, 8.5397212652);
        check("double: da / db", da / db, 1.1557271509925393);
        check("double: da % db", da % db, 0.42330999999999985);

        // byte arithmetic
        byte ba = 5, bb = 3;
        check("byte: ba + bb", ba + bb, 8);
        check("byte: ba - bb", ba - bb, 2);
        check("byte: ba * bb", ba * bb, 15);
        check("byte: ba / bb", ba / bb, 1);
        check("byte: ba % bb", ba % bb, 2);

        // short arithmetic
        short sa = 5, sb = 3;
        check("short: sa + sb", sa + sb, 8);
        check("short: sa - sb", sa - sb, 2);
        check("short: sa * sb", sa * sb, 15);
        check("short: sa / sb", sa / sb, 1);
        check("short: sa % sb", sa % sb, 2);

        // char arithmetic
        char ca = 5, cb = 3;
        check("char: ca + cb", ca + cb, 8);
        check("char: ca - cb", ca - cb, 2);
        check("char: ca * cb", ca * cb, 15);
        check("char: ca / cb", ca / cb, 1);
        check("char: ca % cb", ca % cb, 2);

        // Test rounding errors (float/double)
        float fe = 1.0f / 3.0f;
        check("float: 1.0 / 3.0", fe, 0.33333334f);
        check("float: fe * 3.0", fe * 3.0f, 1.0f);

        double de = 1.0 / 3.0;
        check("double: 1.0 / 3.0", de, 0.3333333333333333);
        check("double: de * 3.0", de * 3.0, 1.0);

        // Test NaN and Infinity (float/double)
        float ff = Float.NaN;
        check("Float.NaN", ff, Float.NaN);
        check("Float.isNaN(ff)", Float.isNaN(ff), true);
        float fg = Float.POSITIVE_INFINITY;
        check("Float.POSITIVE_INFINITY", fg, Float.POSITIVE_INFINITY);

        double df = Double.NaN;
        check("Double.NaN", df, Double.NaN);
        check("Double.isNaN(df)", Double.isNaN(df), true);
        double dg = Double.POSITIVE_INFINITY;
        check("Double.POSITIVE_INFINITY", dg, Double.POSITIVE_INFINITY);

        // Long shift counts use the low six bits; right shift variants differ for negative values.
        long negativeShiftValue = -8L;
        check("long: arithmetic right shift", negativeShiftValue >> 2, -2L);
        check("long: logical right shift", negativeShiftValue >>> 2, 4611686018427387902L);
        check("long: arithmetic shift by 63", negativeShiftValue >> 63, -1L);
        check("long: logical shift by 32", -1L >>> 32, 4294967295L);
        check("int: left shift wraps", Integer.MIN_VALUE << 1, 0);
        check("long: left shift wraps", Long.MIN_VALUE << 1, 0L);

        // Floating-point remainder follows Java's IEEE-754 remainder rules.
        float floatInfinity = Float.POSITIVE_INFINITY;
        double doubleInfinity = Double.POSITIVE_INFINITY;
        check("float: finite remainder by infinity", -5.5f % floatInfinity, -5.5f);
        check("float: infinity remainder", Float.isNaN(floatInfinity % 2.0f), true);
        check("float: large quotient remainder", 1.0e20f % 3.0f, 2.0f);
        check("double: finite remainder by infinity", -5.5 % doubleInfinity, -5.5);
        check("double: infinity remainder", Double.isNaN(doubleInfinity % 2.0), true);
        check("double: large quotient remainder", 1.0e20 % 3.0, 1.0);
        float negativeFloatZero = -0.0f;
        double negativeDoubleZero = -0.0;
        check("float: remainder preserves negative zero",
              1.0f / (negativeFloatZero % 3.0f), Float.NEGATIVE_INFINITY);
        check("double: remainder preserves negative zero",
              1.0 / (negativeDoubleZero % 3.0), Double.NEGATIVE_INFINITY);

        // Integer arithmetic wraps to the Java 32-bit and 64-bit result width.
        int intMaximum = Integer.MAX_VALUE;
        int intMinimum = Integer.MIN_VALUE;
        check("int: addition wraps", intMaximum + 1, intMinimum);
        check("int: subtraction wraps", intMinimum - 1, intMaximum);
        check("int: multiplication wraps", intMaximum * 2, -2);
        check("int: negation wraps", -intMinimum, intMinimum);
        check("int: minimum divided by negative one wraps", intMinimum / -1, intMinimum);
        check("int: minimum remainder negative one", intMinimum % -1, 0);

        long longMaximum = Long.MAX_VALUE;
        long longMinimum = Long.MIN_VALUE;
        check("long: addition wraps", longMaximum + 1L, longMinimum);
        check("long: subtraction wraps", longMinimum - 1L, longMaximum);
        check("long: multiplication wraps", longMaximum * 2L, -2L);
        check("long: negation wraps", -longMinimum, longMinimum);
        check("long: minimum divided by negative one wraps", longMinimum / -1L, longMinimum);
        check("long: minimum remainder negative one", longMinimum % -1L, 0L);
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

    private static void check(String label, float actual, float expected) {
        if (Float.isNaN(expected) && Float.isNaN(actual)) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == NaN" + ConsoleColors.RESET);
        } else if (Float.isInfinite(expected) && expected == actual) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == Infinity" + ConsoleColors.RESET);
        } else if (Math.abs(actual - expected) < 1e-6) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, double actual, double expected) {
        if (Double.isNaN(expected) && Double.isNaN(actual)) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == NaN" + ConsoleColors.RESET);
        } else if (Double.isInfinite(expected) && expected == actual) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == Infinity" + ConsoleColors.RESET);
        } else if (Math.abs(actual - expected) < 1e-12) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, boolean actual, boolean expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + " == " + expected + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " == " + actual + " (expected " + expected + ")" + ConsoleColors.RESET);
        }
    }
}