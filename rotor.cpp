#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;

class Rotor
{
public:
    string mode;
    string frase;
    int posMove;
    int posStep;
    int desloc = 0;
    int numBytesProcessed = 0;
    vector<int> S;

    Rotor() {}
    Rotor(string mode, string frase, int posMove, int posStep) : mode(mode), frase(frase), posMove(posMove), posStep(posStep), S(256)
    {
        for (long unsigned int i = 0; i < S.size(); i++)
        {
            S[i] = i;
        }

        int j = 0;
        for (long unsigned int i = 0; i < S.size(); i++)
        {
            j = (j + S[i] + ((int)frase[i % frase.length()])) % 256;
            int aux = S[i];
            S[i] = S[j];
            S[j] = aux;
        }

        if (mode == "D")
        {
            vector<int> D(256);
            for (int i = 0; i < 255; i++)
            {
                D[S[i]] = i;
            }
            S = D;
        }
    }

    vector<char> process(vector<char> input)
    {
        vector<char> result;

        if (mode == "C")
        {
            for (long unsigned int i = 0; i < input.size(); i++)
            {
                char cifrado = S[(input[i] + desloc + 256) % 256];
                result.push_back(cifrado);

                numBytesProcessed += 1;

                if (numBytesProcessed % posMove == 0)
                {
                    desloc = desloc + posStep % 256;
                }
            }
        }
        else if (mode == "D")
        {

            for (long unsigned int i = 0; i < input.size(); i++)
            {

                int cifrado = (int)input[i];
                if (cifrado < 0)
                {
                    cifrado += 256;
                }

                cifrado = cifrado % 256;

                int decifrado = S[cifrado];

                numBytesProcessed += 1;

                result.push_back((decifrado - desloc + 256) % 256);

                if (numBytesProcessed % posMove == 0)
                {
                    desloc = (desloc + posStep) % 256;
                }
            }
        }

        return result;
    }

    void print()
    {
        cout << frase << " " << mode << endl;
        for (long unsigned int i = 0; i < S.size(); i++)
        {
            cout << S[i] << " ";
            if ((i + 1) % 16 == 0)
            {
                cout << endl;
            }
        }
    }
};

class Cipher
{
public:
    string mode;
    vector<Rotor> rotores;
    int numRotores;
    vector<char> result;

    Cipher(string mode, int numRotores) : mode(mode), numRotores(numRotores) {}

    void process(string inputFile)
    {

        ifstream readInputfile(inputFile, ios::binary);

        vector<char> input;

        char valor;
        while (readInputfile.read(&valor, 1))
        {
            input.push_back(valor);
        }

        readInputfile.close();

        if (mode == "C")
        {
            result = rotores[0].process(input);

            for (int i = 1; i < numRotores; i++)
            {
                result = rotores[i].process(result);
            }
        }
        else if (mode == "D")
        {
            result = rotores.back().process(input);
            for (int i = rotores.size() - 2; i >= 0; i--)
            {
                result = rotores[i].process(result);
            }
        }
    }

    void writeFile(string outputFile)
    {
        ofstream writeOutputFile(outputFile);

        writeOutputFile.write(result.data(), result.size());
    }

    void printRotors()
    {
        string rotorsOutputFile = "rotor-";
        rotorsOutputFile += (char)tolower(mode[0]);
        for (int i = 0; i < numRotores; i++)
        {
            rotorsOutputFile += '-';
            rotorsOutputFile += tolower(rotores[i].frase[0]);
            rotorsOutputFile += '-';
            rotorsOutputFile += to_string(rotores[i].posMove);
            rotorsOutputFile += '-';
            rotorsOutputFile += to_string(rotores[i].posStep);
        }
        rotorsOutputFile += ".txt";

        ofstream rotorOutputStream(rotorsOutputFile);

        for (int i = 0; i < numRotores; i++)
        {
            rotorOutputStream << "Rotor  " << i << ":" << endl;
            for (int j = 0; j < 256; j++)
            {
                rotorOutputStream << right << setw(3) << rotores[i].S[j] << " ";
                if ((j + 1) % 16 == 0)
                {
                    rotorOutputStream << endl;
                }
            }
        }
    }

    void print()
    {
        for (long unsigned int i = 0; i < result.size(); i++)
        {
            cout << std::hex << std::setw(2) << std::setfill('0')
                 << uint32_t(uint8_t(result[i])) << " ";
            if ((i + 1) % 16 == 0)
            {
                cout << endl;
            }
        }
        cout << endl;
    }
};

int main(int argv, char **argc)
{
    string mode = argc[1];
    int numRotores = stoi(argc[2]);

    Cipher c(mode, numRotores);

    vector<string> frases;

    int i = 3;
    for (i=i;i < 3 + numRotores + 1; i++)
    {
        frases.push_back((string)argc[i]);
    }

    int indexFrase = 0;
    int t = i + numRotores * 2;
    for (i = i - 1; i < t - 1; i += 2)
    {
        c.rotores.push_back(Rotor(mode, frases[indexFrase], stoi(argc[i]), stoi(argc[i + 1])));
        indexFrase += 1;
    }

    const char *inputFile = argc[i];
    const char *outputFile = argc[++i];

    c.printRotors();
    c.process(inputFile);
    c.writeFile(outputFile);
}