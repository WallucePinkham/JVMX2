public class TestObjectCreation {
    public static void main(String[] args) {
        // Test object creation
        MyClass obj1 = new MyClass();
        MyClass obj2 = new MyClass(10);
        System.out.println("obj1.x = " + obj1.getX());
        System.out.println("obj2.x = " + obj2.getX());
    }
}

class MyClass {
    private int x;

    public MyClass() {
        this.x = 0;
    }

    public MyClass(int x) {
        this.x = x;
    }

    public int getX() {
        return x;
    }
}
