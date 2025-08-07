public class TestStringBuilder {
    public static void main(String[] args) {
        
        // Test basic StringBuilder operations
        StringBuilder sb = new StringBuilder();
        sb.append("Hello");
        sb.append(" ");
        sb.append("World");
        check("StringBuilder content", sb.toString(), "Hello World");

        // Test insert operation
        sb.insert(5, ",");
        check("After insert", sb.toString(), "Hello, World");

        // Test delete operation
        sb.delete(5, 6);
        check("After delete", sb.toString(), "Hello World");

        // Test replace operation
        sb.replace(0, 5, "Hi");
        check("After replace", sb.toString(), "Hi World");

        // Test length and capacity
        check("Length", sb.length(), 7);
        int initialCapacity = sb.capacity();
        System.out.println("Capacity: " + initialCapacity);

        // Test reverse operation
        sb.reverse();
        check("After reverse", sb.toString(), "dlroW iH");

        // Test substring operation
        String substring = sb.substring(0, 5);
        check("Substring", substring, "dlroW");

        // Test character at specific index
        char charAtIndex = sb.charAt(0);
        check("Character at index 0", charAtIndex, 'd');

        // Test converting to String
        String str = sb.toString();
        check("Converted to String", str, "dlroW iH");

        // Test capacity increase
        sb.ensureCapacity(100);
        check("Capacity after ensureCapacity(100)", sb.capacity() >= 100, true);

        // Test clear operation
        sb.setLength(0);
        check("After clear", sb.toString(), "");
        check("Length after clear", sb.length(), 0);

        // Test appending different data types
        sb.append(123);
        sb.append(45.67);
        sb.append(true);
        check("After appending different data types", sb.toString(), "12345.67true");

        // Test appending a character array
        char[] charArray = {'A', 'B', 'C'};
        sb.append(charArray);
        check("After appending character array", sb.toString(), "12345.67trueABC");

        // Test appending a StringBuilder
        StringBuilder sb2 = new StringBuilder(" Appended StringBuilder");
        sb.append(sb2);
        check("After appending StringBuilder", sb.toString(), "12345.67trueABC Appended StringBuilder");

        // Test capacity after multiple appends
        System.out.println("Final capacity: " + sb.capacity());

        // Test indexOf and lastIndexOf
        int indexOfHello = sb.indexOf("Hello");
        int lastIndexOfWorld = sb.lastIndexOf("World");
        check("Index of 'Hello'", indexOfHello, -1);
        check("Last index of 'World'", lastIndexOfWorld, -1);

        // Test indexOf and lastIndexOf with non-existing substring
        int indexOfNonExisting = sb.indexOf("NonExisting");
        check("Index of 'NonExisting'", indexOfNonExisting, -1);

        // Test capacity after various operations
        System.out.println("Final capacity after various operations: " + sb.capacity());

        // Test StringBuilder with special characters
        StringBuilder specialChars = new StringBuilder();
        specialChars.append("Special characters: !@#$%^&*()");
        check("Special characters", specialChars.toString(), "Special characters: !@#$%^&*()");

        // Test StringBuilder with Unicode characters
        StringBuilder unicodeChars = new StringBuilder();
        unicodeChars.append("Unicode characters: \u03A9 \u03B1 \u03B2");
        check("Unicode characters", unicodeChars.toString(), "Unicode characters: \u03A9 \u03B1 \u03B2");

        // Test StringBuilder with empty content
        StringBuilder emptySb = new StringBuilder();
        check("Empty StringBuilder content", emptySb.toString(), "");

        // Test StringBuilder with null content
        StringBuilder nullSb = new StringBuilder();
        nullSb.append((String) null); // Appends "null"
        check("Null StringBuilder content", nullSb.toString(), "null");

        // Test StringBuilder with large content
        System.out.println("Testing large StringBuilder content...");
        StringBuilder largeSb = new StringBuilder();
        for (int i = 0; i < 10000; i++) {
            if (i % 1000 == 0) {
                System.out.println("Appending line " + i);
            }
            largeSb.append("Line ").append(i).append(" |");
        }
        String first100 = largeSb.toString().substring(0, 100);
        check("Large StringBuilder content (first 100 chars)", first100, "Line 0 |Line 1 |Line 2 |Line 3 |Line 4 |Line 5 |Line 6 |Line 7 |Line 8 |Line 9 |Line 10 |Line 11 |Line 12 |Line 13 |");
        check("Large StringBuilder length", largeSb.length(), 70000);
        System.out.println("Large StringBuilder capacity: " + largeSb.capacity());

        // Test StringBuilder with different initial capacity
        StringBuilder initialCapacitySb = new StringBuilder(50);
        check("Initial capacity", initialCapacitySb.capacity(), 50);

        // Test StringBuilder with append method chaining
        StringBuilder chainedSb = new StringBuilder()
            .append("Chained ")
            .append("StringBuilder ")
            .append("operations.");
        check("Chained StringBuilder", chainedSb.toString(), "Chained StringBuilder operations.");
    }

    private static void check(String label, String actual, String expected) {
        if ((actual == null && expected == null) || (actual != null && actual.equals(expected))) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " (got: '" + actual + "', expected: '" + expected + "')" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, int actual, int expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " (got: " + actual + ", expected: " + expected + ")" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, char actual, char expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " (got: '" + actual + "', expected: '" + expected + "')" + ConsoleColors.RESET);
        }
    }

    private static void check(String label, boolean actual, boolean expected) {
        if (actual == expected) {
            System.out.println(ConsoleColors.GREEN + "SUCCESS: " + label + ConsoleColors.RESET);
        } else {
            System.out.println(ConsoleColors.RED + "FAIL: " + label + " (got: " + actual + ", expected: " + expected + ")" + ConsoleColors.RESET);
        }
    }
}