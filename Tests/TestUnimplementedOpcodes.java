public class TestUnimplementedOpcodes {
    public static void main(String[] args) {
        System.out.println("Running TestUnimplementedOpcodes...");

        if (args.length > 0 && "--force-check-failure".equals(args[0])) {
            check("forced failure propagation", false);
        }

        // 1. Long negation (lneg)
        long l = 123L;
        long negL = -l;
        check("long negation (lneg)", negL == -123L);

        // 2. Float negation (fneg)
        float f = 123.45f;
        float negF = -f;
        check("float negation (fneg)", Math.abs(negF - (-123.45f)) < 1e-3);

        // 3. Long unsigned right shift (lushr)
        long lushrVal = -1L >>> 4;
        check("long unsigned right shift (lushr)", lushrVal == 1152921504606846975L);

        // 4. Long to float conversion (l2f)
        long lVal = 12345L;
        float l2fVal = (float) lVal;
        check("long to float conversion (l2f)", Math.abs(l2fVal - 12345.0f) < 1e-3);

        // 5. Float to long conversion (f2l)
        float fVal = 12345.67f;
        long f2lVal = (long) fVal;
        check("float to long conversion (f2l)", f2lVal == 12345L);

        // 6. Double to long conversion (d2l)
        double dVal = 987654.321;
        long d2lVal = (long) dVal;
        check("double to long conversion (d2l)", d2lVal == 987654L);

        long[] longValues = new long[1];
        long assignedLong = (longValues[0] = 1234567890123L);
        check("dup2_x2 preserves long array assignment value",
              assignedLong == 1234567890123L && longValues[0] == assignedLong);

        double[] doubleValues = new double[1];
        double assignedDouble = (doubleValues[0] = -9876.5);
        check("dup2_x2 preserves double array assignment value",
              assignedDouble == -9876.5 && doubleValues[0] == assignedDouble);

        check("goto_w branches forward when condition is false",
              TestWideBranches.testGotoWide(false) == 0);
        check("goto_w reaches the end of a long branch",
              TestWideBranches.testGotoWide(true) == 12000);
        check("goto_w branches backward with a signed offset",
              TestWideBranches.testGotoWideBackward() == 24000);
        check("jsr_w returns to the instruction after the branch",
              TestWideSubroutine.testWideJsr() == 42);

        System.out.println("TestUnimplementedOpcodes finished.");
    }

    private static void check(String label, boolean condition) {
        if (condition) {
            System.out.println("SUCCESS: " + label);
        } else {
            System.out.println("FAIL: " + label);
            System.exit(1);
        }
    }
}
