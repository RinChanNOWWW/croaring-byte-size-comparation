#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

// External C functions from v1 and v2 test libraries
extern "C" {
size_t test_roaring_v1(const uint64_t *values, size_t count, bool portable);
size_t test_roaring_v2(const uint64_t *values, size_t count, bool portable);
}

void print_header(const std::string &title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void compare_size(const std::vector<uint64_t> &values, const std::string &description, bool portable = true) {
    size_t v1_size = test_roaring_v1(values.data(), values.size(), portable);
    size_t v2_size = test_roaring_v2(values.data(), values.size(), portable);

    std::cout << "\n[" << description << "]" << std::endl;
    std::cout << "  Elements: " << values.size() << std::endl;
    std::cout << "  Portable: " << (portable ? "Yes" : "No") << std::endl;
    std::cout << "  V1 getSizeInBytes: " << v1_size << " bytes" << std::endl;
    std::cout << "  V2 getSizeInBytes: " << v2_size << " bytes" << std::endl;
    std::cout << "  Difference: " << static_cast<int64_t>(v2_size) - static_cast<int64_t>(v1_size) << " bytes (" << std::fixed
              << std::setprecision(2) << (v1_size > 0 ? (static_cast<double>(v2_size) / v1_size * 100.0 - 100.0) : 0.0) << "%)"
              << std::endl;
    std::cout << "  Match: " << (v1_size == v2_size ? "✓ YES" : "✗ NO") << std::endl;
}

int main() {
    print_header("Roaring Bitmap v1 vs v2 - getSizeInBytes Comparison");

    std::cout << "\nThis program compares the getSizeInBytes() method between" << std::endl;
    std::cout << "Roaring Bitmap v1 (0.3.1) and v2 (4.4.2)" << std::endl;

    // Test 1: Empty bitmap
    {
        std::vector<uint64_t> values;
        compare_size(values, "Test 1: Empty Bitmap");
    }

    // Test 2: Single element
    {
        std::vector<uint64_t> values = {42};
        compare_size(values, "Test 2: Single Element");
    }

    // Test 3: Small dense range
    {
        std::vector<uint64_t> values;
        for (uint64_t i = 0; i < 100; ++i) {
            values.push_back(i);
        }
        compare_size(values, "Test 3: Small Dense Range (0-99)");
    }

    // Test 4: Large dense range
    {
        std::vector<uint64_t> values;
        for (uint64_t i = 0; i < 10000; ++i) {
            values.push_back(i);
        }
        compare_size(values, "Test 4: Large Dense Range (0-9999)");
    }

    // Test 5: Sparse data
    {
        std::vector<uint64_t> values;
        for (uint64_t i = 0; i < 1000; ++i) {
            values.push_back(i * 1000);
        }
        compare_size(values, "Test 5: Sparse Data (gaps of 1000)");
    }

    // Test 6: Very sparse data
    {
        std::vector<uint64_t> values;
        for (uint64_t i = 0; i < 100; ++i) {
            values.push_back(i * 1000000);
        }
        compare_size(values, "Test 6: Very Sparse Data (gaps of 1M)");
    }

    // Test 7: Mixed pattern
    {
        std::vector<uint64_t> values;
        // Dense part
        for (uint64_t i = 0; i < 1000; ++i) {
            values.push_back(i);
        }
        // Sparse part
        for (uint64_t i = 0; i < 100; ++i) {
            values.push_back(100000 + i * 1000);
        }
        compare_size(values, "Test 7: Mixed Pattern (dense + sparse)");
    }

    // Test 8: Large values (using high bits)
    {
        std::vector<uint64_t> values;
        for (uint64_t i = 0; i < 1000; ++i) {
            values.push_back((1ULL << 32) + i);  // Values in second 32-bit bucket
        }
        compare_size(values, "Test 8: Large Values (high 32 bits set)");
    }

    // Test 9: Multiple buckets
    {
        std::vector<uint64_t> values;
        for (uint64_t bucket = 0; bucket < 5; ++bucket) {
            for (uint64_t i = 0; i < 100; ++i) {
                values.push_back((bucket << 32) + i);
            }
        }
        compare_size(values, "Test 9: Multiple Buckets (5 buckets, 100 each)");
    }

    // Test 10: Non-portable format
    {
        std::vector<uint64_t> values;
        for (uint64_t i = 0; i < 10000; ++i) {
            values.push_back(i);
        }
        compare_size(values, "Test 10: Large Dense Range (non-portable)", false);
    }

    // Test 11: Random-like pattern
    {
        std::vector<uint64_t> values;
        uint64_t val = 1;
        for (int i = 0; i < 1000; ++i) {
            values.push_back(val);
            val = (val * 1103515245 + 12345) & 0x7FFFFFFF;  // Simple LCG
        }
        compare_size(values, "Test 11: Pseudo-random Pattern");
    }

    // Test 12: Powers of 2
    {
        std::vector<uint64_t> values;
        for (int i = 0; i < 40; ++i) {
            values.push_back(1ULL << i);
        }
        compare_size(values, "Test 12: Powers of 2");
    }

    print_header("Summary");
    std::cout << "\nComparison completed!" << std::endl;
    std::cout << "If sizes differ, it may indicate:" << std::endl;
    std::cout << "  - Different compression algorithms" << std::endl;
    std::cout << "  - Different container implementations" << std::endl;
    std::cout << "  - Serialization format changes between versions" << std::endl;
    std::cout << "\nNote: Both versions use the same high-level algorithm but" << std::endl;
    std::cout << "      implementation details may vary." << std::endl;

    return 0;
}
