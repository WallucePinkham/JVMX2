public class TestStringBuilder {
    public static void main(String[] args) {
        // Test basic StringBuilder operations
        StringBuilder sb = new StringBuilder();
        sb.append("Hello");
        sb.append(" ");
        sb.append("World");
        System.out.println("StringBuilder content: " + sb.toString());

        // Test insert operation
        sb.insert(5, ",");
        System.out.println("After insert: " + sb.toString());

        // Test delete operation
        sb.delete(5, 6);
        System.out.println("After delete: " + sb.toString());

        // Test replace operation
        sb.replace(0, 5, "Hi");
        System.out.println("After replace: " + sb.toString());

        // Test length and capacity
        System.out.println("Length: " + sb.length());
        System.out.println("Capacity: " + sb.capacity());

        // Test reverse operation
        sb.reverse();
        System.out.println("After reverse: " + sb.toString());

        // Test substring operation
        String substring = sb.substring(0, 5);
        System.out.println("Substring: " + substring);

        // Test character at specific index
        char charAtIndex = sb.charAt(0);
        System.out.println("Character at index 0: " + charAtIndex);

        // Test converting to String
        String str = sb.toString();
        System.out.println("Converted to String: " + str);

        // Test capacity increase
        sb.ensureCapacity(100);
        System.out.println("Capacity after ensureCapacity(100): " + sb.capacity());
        // Test clear operation
        sb.setLength(0);
        System.out.println("After clear: " + sb.toString());
        System.out.println("Length after clear: " + sb.length());

        // Test appending different data types
        sb.append(123);
        sb.append(45.67);
        sb.append(true);
        System.out.println("After appending different data types: " + sb.toString());

        // Test appending a character array
        char[] charArray = {'A', 'B', 'C'};
        sb.append(charArray);
        System.out.println("After appending character array: " + sb.toString());

        // Test appending a StringBuilder
        StringBuilder sb2 = new StringBuilder(" Appended StringBuilder");
        sb.append(sb2);
        System.out.println("After appending StringBuilder: " + sb.toString());
        // Test capacity after multiple appends
        System.out.println("Final capacity: " + sb.capacity());
        // Test indexOf and lastIndexOf
        int indexOfHello = sb.indexOf("Hello");
        int lastIndexOfWorld = sb.lastIndexOf("World");
        System.out.println("Index of 'Hello': " + indexOfHello);
        System.out.println("Last index of 'World': " + lastIndexOfWorld);

        // Test indexOf and lastIndexOf with non-existing substring
        int indexOfNonExisting = sb.indexOf("NonExisting");
        System.out.println("Index of 'NonExisting': " + indexOfNonExisting);

        // Test capacity after various operations
        System.out.println("Final capacity after various operations: " + sb.capacity());

        // Test StringBuilder with special characters
        StringBuilder specialChars = new StringBuilder();
        specialChars.append("Special characters: !@#$%^&*()");
        System.out.println(specialChars.toString());
        // Test StringBuilder with Unicode characters
        StringBuilder unicodeChars = new StringBuilder();
        unicodeChars.append("Unicode characters: \u03A9 \u03B1 \u03B2");
        System.out.println(unicodeChars.toString());
        // Test StringBuilder with empty content
        StringBuilder emptySb = new StringBuilder();
        System.out.println("Empty StringBuilder content: " + emptySb.toString());
        // Test StringBuilder with null content
        StringBuilder nullSb = new StringBuilder(); // This will not throw an exception, but will create an empty StringBuilder
        nullSb.append((String) null); // This will append "null" as a string
        System.out.println("Null StringBuilder content: " + nullSb.toString());
        // Test StringBuilder with large content
        StringBuilder largeSb = new StringBuilder();
        for (int i = 0; i < 10000; i++) {
            largeSb.append("Line ").append(i).append("\n");
        }
        System.out.println("Large StringBuilder content (first 100 chars): " + largeSb.toString().substring(0, 100));
        System.out.println("Large StringBuilder length: " + largeSb.length());
        System.out.println("Large StringBuilder capacity: " + largeSb.capacity());
        // Test StringBuilder with different initial capacity
        StringBuilder initialCapacitySb = new StringBuilder(50);
        System.out.println("Initial capacity: " + initialCapacitySb.capacity());

        // Test StringBuilder with append method chaining
        StringBuilder chainedSb = new StringBuilder()
            .append("Chained ")
            .append("StringBuilder ")
            .append("operations.");
    }
}