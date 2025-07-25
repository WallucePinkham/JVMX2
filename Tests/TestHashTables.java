import java.util.HashMap;
import java.util.Map;

public class TestHashTables {
    private HashMap<String, Integer> hashTable;

    public TestHashTables() {
        hashTable = new HashMap<>();
    }

    // Method to add key-value pairs to the hash table
    public void add(String key, Integer value) {
        hashTable.put(key, value);
        //System.out.println("Added: " + key + " -> " + value);
    }

    // Method to retrieve a value by key
    public Integer get(String key) {
        Integer value = hashTable.get(key);
        if (value != null) {
            System.out.println("Retrieved: " + key + " -> " + value);
        } else {
            System.out.println("Key not found: " + key);
        }
        return value;
    }

    // Method to remove a key-value pair from the hash table
    public void remove(String key) {
        if (hashTable.containsKey(key)) {
            hashTable.remove(key);
            System.out.println("Removed: " + key);
        } else {
            System.out.println("Key not found: " + key);
        }
    }

    // Method to display the contents of the hash table
    public void display() {
        System.out.println("Hash Table Contents:");
        for (Map.Entry<String, Integer> entry : hashTable.entrySet()) {
            System.out.println(entry.getKey() + " -> " + entry.getValue());
        }
    }

    public static void main(String[] args) {
        TestHashTables test = new TestHashTables();

        // Adding key-value pairs
        test.add("Alice", 30);
        test.add("Bob", 25);
        test.add("Charlie", 35);
        test.add("D", 35);
        test.add("E", 35);
        test.add("F", 35);
        test.add("G", 35);
        test.add("H", 35);
        test.add("I", 35);


        // Displaying the hash table
        test.display();

        // Retrieving values
        // test.get("Alice");
        // test.get("Bob");
        // test.get("David"); // Key not found

        // // Removing a key-value pair
        // test.remove("Bob");
        // test.display();

        // // Attempting to remove a non-existent key
        // test.remove("David");
    }
}
