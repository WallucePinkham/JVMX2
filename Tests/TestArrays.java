public class TestArrays {
    public static void main(String[] args) {
        // Test array creation
        int[] intArray = new int[5];
        String[] stringArray = new String[3];
        
        // Initialize arrays
        for (int i = 0; i < intArray.length; i++) {
            intArray[i] = i * 10;
        }
        stringArray[0] = "Hello";
        stringArray[1] = "World";
        stringArray[2] = "!";

        // Print integer array
        System.out.println("Integer Array:");
        for (int num : intArray) {
            System.out.print(num + " ");
        }
        System.out.println();

        // Print string array
        System.out.println("String Array:");
        for (String str : stringArray) {
            System.out.print(str + " ");
        }
        System.out.println();

        // Test array length
        System.out.println("Length of intArray: " + intArray.length);
        System.out.println("Length of stringArray: " + stringArray.length);
        // Test accessing elements
        System.out.println("First element of intArray: " + intArray[0]);
        System.out.println("First element of stringArray: " + stringArray[0]);
        // Test multi-dimensional array
        int[][] multiArray = {
            {1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };
        System.out.println("Multi-dimensional Array:");
        for (int[] row : multiArray) {
            for (int num : row) {
                System.out.print(num + " ");
            }
            System.out.println();
        }
        // Test array copy
        int[] copiedArray = new int[intArray.length];
        System.arraycopy(intArray, 0, copiedArray, 0, intArray.length);
        System.out.println("Copied Array:");
        for (int num : copiedArray) {
            System.out.print(num + " ");
        }

        
    }
}
