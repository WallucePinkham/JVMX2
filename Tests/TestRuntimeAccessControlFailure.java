public class TestRuntimeAccessControlFailure {
    public static void main(String[] args) {
        System.out.println("Running TestRuntimeAccessControlFailure...");
        boolean exceptionThrown = false;
        try {
            // Simulate or attempt an action where access control should throw
            // IllegalAccessError.
            // Currently, because runtime access checks (JVMS §5.4.4) are
            // incomplete/missing,
            // no exception is thrown here.
        } catch (java.lang.IllegalAccessError e) {
            exceptionThrown = true;
            System.out.println("Caught expected IllegalAccessError.");
        } catch (Throwable t) {
            System.out.println("Caught other throwable: " + t);
        }

        if (!exceptionThrown) {
            throw new RuntimeException(
                    "FAIL: Expected IllegalAccessError was not thrown due to missingime access control enforcement (JVMS §5.4.4).");
        }
        System.out.println("SUCCESS: TestRuntimeAccessControlFailure passed.");
    }
}