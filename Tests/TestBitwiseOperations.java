public class TestBitwiseOperations {
    public static void main(String[] args) {
        int a = 5;  // 0101 in binary
        int b = 3;  // 0011 in binary

        System.out.println("a & b = " + (a & b));  // Bitwise AND
        System.out.println("a | b = " + (a | b));  // Bitwise OR
        System.out.println("a ^ b = " + (a ^ b));  // Bitwise XOR
        System.out.println("~a = " + (~a));        // Bitwise NOT
        System.out.println("a << 1 = " + (a << 1)); // Left shift
        System.out.println("a >> 1 = " + (a >> 1)); // Right shift

        // Test with long values
        long la = 5L;  // 0101 in binary
        long lb = 3L;  // 0011 in binary

        System.out.println("la & lb = " + (la & lb));  // Bitwise AND
        System.out.println("la | lb = " + (la | lb));  // Bitwise OR
        System.out.println("la ^ lb = " + (la ^ lb));  // Bitwise XOR
        System.out.println("~la = " + (~la));        // Bitwise NOT
        System.out.println("la << 1 = " + (la << 1)); // Left shift
        System.out.println("la >> 1 = " + (la >> 1)); // Right shift

        // Test with byte values
        byte ba = 5;  // 0101 in binary
        byte bb = 3;  // 0011 in binary
        System.out.println("ba & bb = " + (ba & bb));  // Bitwise AND
        System.out.println("ba | bb = " + (ba | bb));  // Bitwise OR
        System.out.println("ba ^ bb = " + (ba ^ bb));  // Bitwise XOR
        System.out.println("~ba = " + (~ba));        // Bitwise NOT   

        System.out.println("ba << 1 = " + (ba << 1)); // Left shift
        System.out.println("ba >> 1 = " + (ba >> 1)); // Right shift

        // Test with short values
        short sa = 5;  // 0101 in binary
        short sb = 3;  // 0011 in binary
        System.out.println("sa & sb = " + (sa & sb));  // Bitwise AND
        System.out.println("sa | sb = " + (sa | sb));  // Bitwise OR
        System.out.println("sa ^ sb = " + (sa ^ sb));  // Bitwise XOR
        System.out.println("~sa = " + (~sa));        // Bitwise NOT
        System.out.println("sa << 1 = " + (sa << 1)); // Left shift
        System.out.println("sa >> 1 = " + (sa >> 1)); // Right shift

        // Test with char values
        char ca = 5;  // 0101 in binary
        char cb = 3;  // 0011 in binary
        System.out.println("ca & cb = " + (ca & cb));  // Bitwise AND
        System.out.println("ca | cb = " + (ca | cb));  // Bitwise OR
        System.out.println("ca ^ cb = " + (ca ^ cb));  // Bitwise XOR
        System.out.println("~ca = " + (~ca));        // Bitwise NOT
        System.out.println("ca << 1 = " + (ca << 1)); // Left shift
        System.out.println("ca >> 1 = " + (ca >> 1)); // Right shift
    }
}
