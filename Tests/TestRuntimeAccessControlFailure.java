public class TestRuntimeAccessControlFailure {
    public static void main(String[] args) {
        System.out.println("Running TestRuntimeAccessControlFailure...");
        try {
            AccessTest.main(new String[0]);
        } catch (java.lang.IllegalAccessError e) {
            System.out.println("Caught expected IllegalAccessError.");
            System.out.println("SUCCESS: TestRuntimeAccessControlFailure passed.");
            return;
        } catch (Throwable t) {
            throw new RuntimeException("Expected IllegalAccessError, but caught " + t, t);
        }

        throw new RuntimeException(
                "Expected IllegalAccessError when AccessTest invoked a private method from another class.");
    }

    private static void restrictedMethod() {
    }
}
