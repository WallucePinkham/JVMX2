public class TestFloat {
    public static void main(String[] args) {
        // Test simple arithmetic operations
        float a = 1.0f;
        float b = 2.0f;
        System.out.println("a + b = " + (a + b));
        System.out.println("a - b = " + (a - b));
        System.out.println("a * b = " + (a * b));
        System.out.println("a / b = " + (a / b));

        // Test more complex arithmetic operations
        float c = 3.14159f;
        float d = 2.71828f;
        System.out.println("c + d = " + (c + d));
        System.out.println("c - d = " + (c - d));
        System.out.println("c * d = " + (c * d));
        System.out.println("c / d = " + (c / d));

        // Test rounding errors
        float e = 1.0f / 3.0f;
        System.out.println("1.0f / 3.0f = " + e);
        System.out.println("e * 3.0f = " + (e * 3.0f));

        // Test NaN and Infinity
        float f = Float.NaN;
        System.out.println("Float.NaN = " + f);
        System.out.println("Float.isNaN(f) = " + Float.isNaN(f));

        float g = Float.POSITIVE_INFINITY;
        System.out.println("Float.POSITIVE_INFINITY = " + g);
       
    }
}