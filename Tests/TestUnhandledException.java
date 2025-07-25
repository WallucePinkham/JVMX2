public class TestUnhandledException {
    public static void main(String[] args) {
        System.out.println("About to throw an exception...");

        // This line will throw a RuntimeException
        throw new RuntimeException("This is an unhandled exception!");
    }
}
