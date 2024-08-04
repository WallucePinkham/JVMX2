public class TestLanguageFeatures {

    public static String RedOrGreen(boolean success) {
        if (success) {
            return ConsoleColors.GREEN;
        }

        return ConsoleColors.RED;
    }

    public static void main(String[] args) {
        // Arithmetic operations
        int x = 5;
        int y = 2;
        int expectedSum = 7;
        int actualSum = x + y;
        System.out.println("x + y = " + RedOrGreen(actualSum == expectedSum) + actualSum + ConsoleColors.RESET
                + " (expected: " + expectedSum + ")");

        int expectedDiff = 3;
        int actualDiff = x - y;
        System.out.println("x - y = " + RedOrGreen(actualDiff == expectedDiff) + actualDiff + ConsoleColors.RESET
                + " (expected: " + expectedDiff + ")");

        int expectedProduct = 10;
        int actualProduct = x * y;
        System.out.println("x * y = " + RedOrGreen(actualProduct == expectedProduct) + actualProduct
                + ConsoleColors.RESET + " (expected: " + expectedProduct + ")");

        int expectedQuotient = 2;
        int actualQuotient = x / y;
        System.out.println("x / y = " + RedOrGreen(actualQuotient == expectedQuotient) + actualQuotient
                + ConsoleColors.RESET + " (expected: " + expectedQuotient + ")");

        int expectedRemainder = 1;
        int actualRemainder = x % y;
        System.out.println("x % y = " + RedOrGreen(actualRemainder == expectedRemainder) + actualRemainder
                + ConsoleColors.RESET + " (expected: " + expectedRemainder + ")");

        // Bitwise operations
        int a = 0x0005;
        int b = 0x0002;
        int expectedAnd = 0x0000;
        int actualAnd = a & b;
        System.out.println("a & b = " + RedOrGreen(actualAnd == expectedAnd) + actualAnd + ConsoleColors.RESET
                + " (expected: " + expectedAnd + ")");

        int expectedOr = 0x0007;
        int actualOr = a | b;
        System.out.println("a | b = " + RedOrGreen(actualOr == expectedOr) + actualOr + ConsoleColors.RESET
                + " (expected: " + expectedOr + ")");

        int expectedXor = 0x0007;
        int actualXor = a ^ b;
        System.out.println("a ^ b = " + RedOrGreen(actualXor == expectedXor) + actualXor + ConsoleColors.RESET
                + " (expected: " + expectedXor + ")");

        int expectedNot = -6;
        int actualNot = ~a;
        System.out.println("~a = " + RedOrGreen(actualNot == expectedNot) + actualNot + ConsoleColors.RESET
                + " (expected: " + (int) expectedNot + ")");

        // Logical operations
        boolean c = true;
        boolean d = false;
        boolean expectedAndResult = false;
        boolean actualAndResult = c && d;
        System.out.println("c && d = " + RedOrGreen(actualAndResult == expectedAndResult) + actualAndResult
                + ConsoleColors.RESET + " (expected: " + expectedAndResult + ")");

        boolean expectedOrResult = true;
        boolean actualOrResult = c || d;
        System.out.println("c || d = " + RedOrGreen(actualOrResult == expectedOrResult) + actualOrResult
                + ConsoleColors.RESET + " (expected: " + expectedOrResult + ")");

        boolean expectedNotResult = false;
        boolean actualNotResult = !c;
        System.out.println("!c = " + RedOrGreen(actualNotResult == expectedNotResult) + actualNotResult
                + ConsoleColors.RESET + " (expected: " + expectedNotResult + ")");

        // Assignment operators
        int e = 5;
        e += 2;
        int expectedAddAssign = 7;
        System.out.println("e += 2 = " + RedOrGreen(e == expectedAddAssign) + e + ConsoleColors.RESET + " (expected: "
                + expectedAddAssign + ")");
        // assert e == expectedAddAssign;

        // Assignment operators
        e -= 1;
        int expectedSubAssign = 6;
        System.out.println("e -= 1 = " + RedOrGreen(e == expectedSubAssign) + e + ConsoleColors.RESET + " (expected: "
                + expectedSubAssign + ")");

        e *= 3;
        int expectedMulAssign = 18;
        System.out.println("e *= 3 = " + RedOrGreen(e == expectedMulAssign) + e + ConsoleColors.RESET + " (expected: "
                + expectedMulAssign + ")");

        e /= 2;
        int expectedDivAssign = 9;
        System.out.println("e /= 2 = " + RedOrGreen(e == expectedDivAssign) + e + ConsoleColors.RESET + " (expected: "
                + expectedDivAssign + ")");

        // Control flow
        int f = 5;
        boolean expectedIfResult = false;
        boolean actualIfResult = f > 10;
        System.out.println("f > 10 = " + RedOrGreen(actualIfResult == expectedIfResult) + actualIfResult
                + ConsoleColors.RESET + " (expected: " + expectedIfResult + ")");

        int expectedSwitchResult = 5;
        int actualSwitchResult = f;
        System.out.println("switch (f) = " + RedOrGreen(actualSwitchResult == expectedSwitchResult) + actualSwitchResult
                + ConsoleColors.RESET + " (expected: " + expectedSwitchResult + ")");

        // Methods
        int expectedMethodResult = 5;
        int actualMethodResult = add(2, 3);
        System.out.println("add(2, 3) = " + RedOrGreen(actualMethodResult == expectedMethodResult) + actualMethodResult
                + ConsoleColors.RESET + " (expected: " + expectedMethodResult + ")");

        // Arrays
        int[] arr = { 1, 2, 3, 4, 5 };
        int expectedArrayLength = 5;
        int actualArrayLength = arr.length;
        System.out.println("arr.length = " + RedOrGreen(actualArrayLength == expectedArrayLength) + actualArrayLength
                + ConsoleColors.RESET + " (expected: " + expectedArrayLength + ")");

        int expectedArrayElement = 2;
        int actualArrayElement = arr[1];
        System.out.println("arr[1] = " + RedOrGreen(actualArrayElement == expectedArrayElement) + actualArrayElement
                + ConsoleColors.RESET + " (expected: " + expectedArrayElement + ")");
    }

    public static int add(int x, int y) {
        return x + y;
    }
}
