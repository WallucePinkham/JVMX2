public class TestControlFlow {
    public static void main(String[] args) {
        // Test if-else
        int a = 5;
        if (a > 0) {
            System.out.println("a is positive");
        } else {
            System.out.println("a is non-positive");
        }

        // Test switch
        int b = 2;
        switch (b) {
            case 1:
                System.out.println("b is 1");
                break;
            case 2:
                System.out.println("b is 2");
                break;
            default:
                System.out.println("b is neither 1 nor 2");
        }

        // Test for loop
        for (int i = 0; i < 5; i++) {
            System.out.println("i = " + i);
        }

        // Test while loop
        int j = 0;
        while (j < 5) {
            System.out.println("j = " + j);
            j++;
        }

        // Test do-while loop
        int k = 0;
        do {
            System.out.println("k = " + k);
            k++;
        } while (k < 5);
    }
}
