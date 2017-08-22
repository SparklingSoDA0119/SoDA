#define MAX_LENGTH 32
int getIntegerComplemetn(int n)
{
    n = -1;
    int inputNum = n;
    int retVal = 0;

    if (inputNum < 0 || inputNum > 100000) {
        return 0xFFFFFFFF;
    }

    int i = 0;
    int bitstream[MAX_LENGTH] = {0, };

    for (i = 0; inputNum > 0; i++) {
        bitstream[i] = inputNum % 2;
        inputNum /= 2;
    }

    int bitLength = i;
    int j = i;
    for (j--; j < bitLength; i++) {
        if (bitstream[i] == 0) {
            bitstream[i] = 1;
        }
        else {
            bitstream[i] = 0;
        }
    }

    i = bitLength;
    for (i--; i >= 0; --i) {
        retVal += bitstream[i] * ((int)pow(2.0, (double)i));
        cout << bitstream[i];
    }
    cout << endl << "retVal = " << retVal;

    return retVal;
}
