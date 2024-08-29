/**
 * Compile with `g++ binary_check.cpp canvas.cpp -lgmpxx -lgmp -o canvas`. Make sure to
 * `sudo apt install libgmp-dev`.
 */

#include "binary_check.h"
#include <algorithm>
#include <cmath>
#include <gmpxx.h>
#include <iostream>

const int BIN_CHECK_TRIAL_COUNT = 1000;
mpf_class* factorialMemoize;
mpf_class** binCheckMemoize;

mpf_class factorial(int n) {
    if (n == 1) {
        return n;
    }
    if (factorialMemoize[n] == NULL) {
        factorialMemoize[n] = n * factorial(n - 1);
    }
    return factorialMemoize[n];
}

mpf_class combination(int n, int r) {
    if (r == 0 || n == r) {
        return 1;
    }
    
    return factorial(n) / (factorial(r) * factorial(n - r));
}

mpf_class genTree(
        int k,
        int optionCount,
        int branchCount,
        int scoreOld,
        mpf_class attemptCountOld,
        mpf_class probOld,
        int depth) {
    if (branchCount == NULL) {
        branchCount = k + 1;
    }
    if (depth == optionCount - 1) {
        branchCount = 1;
    }

    mpf_class ev = 0.0;
    for (int j = 0; j < branchCount; j++) { // `j` is the red number
        int incorrectBefore = k - scoreOld; // `i` (except for attempt 1 on the tree)
        int scoreNew = k - j;
        mpf_class newAttemptCount = attemptCountOld;
        mpf_class probNew;
        if (depth == optionCount - 1) {     // if on fourth non-extra attempt
            probNew = probOld;
        } else {
            newAttemptCount++;                  // remember that fourth attempts are overlapped and do not count
            probNew = probOld \
                    * combination(incorrectBefore, incorrectBefore - j) \
                    * pow((float) 1 / (optionCount - depth), incorrectBefore - j) \
                    * pow((float) (optionCount - depth - 1) / (optionCount - depth), j);
        }

        // if we've reached an ending, calculate attempts * total prob and add to ev
        if (scoreNew == k) {
            ev += newAttemptCount * probNew;
            continue;
        }

        // binary check simulator (don't forget symmetry!)
        int a = std::min(scoreNew - scoreOld, k - (scoreNew - scoreOld));
        int b = k - scoreOld;
        if (a != 0 && b != 1) { // if we need binary check
            if (binCheckMemoize[b][a] == NULL) {
                if (a == 1 && (b & (b - 1) == 0) && b != 0) {
                    // if single-target and `b` is a power of 2 (no rounding), take shortcut    
                    // (https://stackoverflow.com/a/57025941)
                    binCheckMemoize[b][a] = log2(b);
                } else {
                    int binCheckAttemptCountTotal = 0;
                    for (int i = 0; i < BIN_CHECK_TRIAL_COUNT; i++) {
                        Node tree(a, b, NULL, NULL);
                        binCheckAttemptCountTotal += tree.runBinCheck();
                    }
                    binCheckMemoize[b][a] = (float) binCheckAttemptCountTotal / BIN_CHECK_TRIAL_COUNT;
                }
            }
            newAttemptCount += binCheckMemoize[b][a];
        }

        // recursively call on sub-branches
        // `j + 1` to account for getting 0 more correct next attempt
        ev += genTree(k, optionCount, j + 1, scoreNew, newAttemptCount, probNew, depth + 1);
    }

    return ev;
}

int main() {
    int kMin;
    int kMax;
    int optionCount;
    std::cout << "k min (inclusive): ";
    std::cin >> kMin;
    std::cout << "k max (exclusive): ";
    std::cin >> kMax;
    std::cout << "number of options per question: ";
    std::cin >> optionCount;

    factorialMemoize = (mpf_class*) malloc(kMax * sizeof(factorialMemoize[0])); // using `kMax` cause lazy
    for (int i = 0; i < kMax; i++) {
        // "conditional jump or move depends on uninitialized value" there are no memory errors in ba sing se
        // i dont think im initializing `mpf_class`es right, because there are no memory issues if they're `double`s
        factorialMemoize[i] = mpf_class(0.0);
    }
    binCheckMemoize = (mpf_class**) malloc(kMax * sizeof(binCheckMemoize[0]));
    for (int i = 0; i < kMax; i++) {
        binCheckMemoize[i] = (mpf_class*) malloc(kMax * sizeof(binCheckMemoize[0][0]));
        for (int j = 0; j < kMax; j++) {
            binCheckMemoize[i][j] = mpf_class(0.0);
        }
    }

    for (int k = kMin; k < kMax; k++) {
        std::cout << k << " " << genTree(k, optionCount, NULL, 0, 0, 1.0, 0) / k << "\n";
    }

    // free
    for (int i = 0; i < kMax; i++) {
        free(binCheckMemoize[i]);
    }
    free(binCheckMemoize);
    free(factorialMemoize);
}
