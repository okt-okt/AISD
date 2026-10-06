#define _MAIN_FILE_
#include <chrono>
#include "compress.h"
#include <sstream>
#include <string>




int main()
{
    setlocale(LC_ALL, "Russian");
    std::wstring out_file;
    dataset* d {}, * b {}, * g {}, * a {}, * c {}, * e {}, * nn[2] {};
    uint32_t buf;
    FILE* f {};
    const wchar_t* compressor_name {}, * stat_file {}, * uncompressed = L" UNCOMPRESSED";
    int hr = 0, var;
    wchar_t line[100];
    bool include_file[9] {};

    auto compression_rate = [](size_t orig, size_t compr)
        {
            return 100.f * (orig > compr ? float(orig - compr) : -float(compr - orig)) / float(orig);
        };
#define time_point(i) times[i] = std::chrono::high_resolution_clock::now()
#define milliseconds(i,j) std::chrono::duration_cast<std::chrono::milliseconds>(times[j]-times[i]).count()
#define dataset_size(q) (q->first.size()+q->second.size())

    const wchar_t* test_files[] = { L"page11-300.aisd.raw", L"mountains.aisd.raw", L"borzoi-4.aisd.raw", L"dbghelp.dll", L"enwik7.txt", L"Oblomov 0.txt", L"Oblomov 8.txt", L"Wool.txt", L"SIMPLE.TXT" };
    const uint8_t symb_size[9] = { 1, 1, 3, 1, 0, 1, 0, 1, 1 };
    const wchar_t* test_files_names[] = { L"page11-300", L"mountains", L"borzoi-4", L"dbghelp", L"enwik7", L"Oblomov 0", L"Oblomov 8", L"Wool", L"SIMPLE" };
    const wchar_t* test_files_ext[] = { L".aisd.raw", L".aisd.raw", L".aisd.raw", L".dll", L".txt", L".txt", L".txt", L".txt", L".TXT" };
    const wchar_t* compressors[] = { L"C", L"R", L"BR", L"BMC", L"BMRC", L"S", L"SC", L"W", L"WC"};


    if (true)
    {
        puts("Test files:");
        for (int file = 0; file < 9; ++file)
            wprintf(L" %c - %s\n", (file + '0'), test_files[file]);
        printf("Input letter codes for files you would like to include in testing: ");
        wscanf_s(L"%s", line, 100);
        for (int i = 0; i < 100 && line[i]; ++i)
            if (L'0' <= line[i] && line[i] < L'9')
                include_file[line[i] - L'0'] = true;
        printf("Input statistics file to output information to (no spaces!): ");
        wscanf_s(L"%s", line, 100);
        for (stat_file = line; *stat_file == L' '; ++stat_file);
    }

    if (true)
    {
        if (_wfopen_s(&f, stat_file, L"wt"))
            return -100;
        for (int file = 0; file < 9; ++file) if(include_file[file])
        {
        #define file_size(ds) (dataset_size(ds) - (ds->first.size() && ds->first[0] == TYPE_Text ? 3 : 0))
            d = LoadFromFile(test_files[file]);
            fwprintf(f, L"\\noindent\\begin{tabularx}{\\textwidth}{|>{\\centering\\arraybackslash}m{1.3cm}|>{\\centering\\arraybackslash}X|>{\\centering\\arraybackslash}m{4.5cm}|>{\\centering\\arraybackslash}X|>{\\centering\\arraybackslash}X|>{\\centering\\arraybackslash}X|}\n\\hline\n\\multirow{9}{*}{\"%s\"} & \\multirow{9}{*}{%llu}\n & ", test_files[file], file_size(d));

        #define actual ((c-compressor)&1)
            for (auto compressor : compressors)
            {
                nn[1] = new dataset(*d);
                const wchar_t* c = compressor;
                wprintf(L"Testing \"%s\" for %s: ", test_files[file], compressor);
                std::wstringstream name;
                name << L"folder\\" << test_files_names[file] << L' ';
                while (*c)
                {
                    switch (*c)
                    {
                        case 'R':
                            nn[actual] = CompressRLE(nn[1 - actual], 1, (symb_size[file] ? symb_size[file] : 1));
                            name << L"RLE"; fwprintf(f, L"RLE"); putchar('R');
                            break;
                        case 'H':
                            nn[actual] = CompressHA(nn[1 - actual], false);
                            name << L"HA"; fwprintf(f, L"HA"); putchar('H');
                            break;
                        case 'C':
                            nn[actual] = CompressHA(nn[1 - actual]);
                            name << L"HAC"; fwprintf(f, L"HAC"); putchar('C');
                            break;
                        case '7':
                            nn[actual] = CompressLZ77(nn[1 - actual], 0xFFFF);
                            name << L"LZ77"; fwprintf(f, L"LZ77"); putchar('7');
                            break;
                        case 'S':
                            nn[actual] = CompressLZSS(nn[1 - actual], 32767);
                            name << L"LZSS"; fwprintf(f, L"LZSS"); putchar('S');
                            break;
                        case '8':
                            nn[actual] = CompressLZ78(nn[1 - actual], 46000);
                            name << L"LZ78"; fwprintf(f, L"LZ78"); putchar('8');
                            break;
                        case 'W':
                            nn[actual] = CompressLZW(nn[1 - actual], 46000);
                            name << L"LZW"; fwprintf(f, L"LZW"); putchar('W');
                            break;
                        case 'M':
                            nn[actual] = TransformMTF(nn[1 - actual]);
                            name << L"MTF"; fwprintf(f, L"MTF"); putchar('M');
                            break;
                        case 'B':
                            nn[actual] = TransformBWT(nn[1 - actual], 3200);
                            name << L"BWT"; fwprintf(f, L"BWT"); putchar('B');
                            break;
                        default:
                            nn[actual] = new dataset(*nn[1 - actual]);
                            name << L"?"; fwprintf(f, L"?"); putchar('-');
                    }
                    if (*(++c))
                        (name << L'+'), fwprintf(f, L"+");
                    delete nn[actual];
                }
                putchar(' ');
                g = new dataset(*nn[1 - actual]);
                do
                {
                    switch (*(--c))
                    {
                        case 'R':
                            nn[1 - actual] = UncompressRLE(nn[actual]);
                            putchar('R');
                            break;
                        case 'H':
                            nn[1 - actual] = UncompressHA(nn[actual]);
                            putchar('H');
                            break;
                        case 'C':
                            nn[1 - actual] = UncompressHA(nn[actual]);
                            putchar('C');
                            break;
                        case '7':
                            nn[1 - actual] = UncompressLZ77(nn[actual]);
                            putchar('7');
                            break;
                        case 'S':
                            nn[1 - actual] = UncompressLZSS(nn[actual]);
                            putchar('S');
                            break;
                        case '8':
                            nn[1 - actual] = UncompressLZ78(nn[actual]);
                            putchar('8');
                            break;
                        case 'W':
                            nn[1 - actual] = UncompressLZW(nn[actual]);
                            putchar('W');
                            break;
                        case 'M':
                            nn[1 - actual] = DetransformMTF(nn[actual]);
                            putchar('M');
                            break;
                        case 'B':
                            nn[1 - actual] = DetransformBWT(nn[actual]);
                            putchar('B');
                            break;
                        default:
                            nn[1 - actual] = new dataset(*nn[actual]);
                            putchar('-');
                    }
                    delete nn[actual];
                }
                while (c != compressor);

                //for(uint32_t i = 0; i < 0x109ae5; ++i)
                //    if (d->second[i] != nn[1]->second[i])
                //    {
                //        printf("%u ", i);
                //    }

                if (*d == *nn[1 - actual])
                    printf(" success\n");
                else
                {
                    printf(" fail\n");
                    goto FAIL;
                }

                SaveToFile(g, (name.str() + L".aisd.raw").c_str());
                SaveToFile(nn[1-actual], (name.str() + uncompressed + test_files_ext[file]).c_str());

                fwprintf(f, L" & %llu & %llu & %.1f %s", file_size(g), file_size(nn[1-actual]), compression_rate(file_size(d), file_size(g)), (compressor == compressors[sizeof(compressors)/sizeof(compressor)-1] ? L"\\end{tabularx}\n\n" : L"\\\\\\cline{3-6}\n&& "));
                fflush(f);

                delete g; delete nn[1 - actual];
            #undef actual
            }
            delete d;
        }

    FAIL:
        fclose(f);
        return -hr;
    }

    const int k = 32;
    const wchar_t* test_for = L"HA";
    int X[] = { 1, 1 }, Y[] = { 1, 1 }, Z[] = { 0, 0 }; // from start to end
    int X0[] = {  63000,62000,60000, 46000,45000,30000,20000, 12000, 5120, 1024,256,   }, Y0[] = { 1 }, Z0[] = { 1 }; // iterating values
    std::chrono::high_resolution_clock::time_point times[10];

    if (_wfopen_s(&f, stat_file, L"wt"))
        return -100;
    fwprintf(f, L"%s\n", L"file;original size;HA compressed size;HA meta size;HA2 compressed size;HA2 meta size;HA compression rate;HA2 compression rate;HA compression time;HA uncompression time;HA fraction;HA2 compression time;HA2 uncompression time;HA2 fraction");
    //fwprintf(f, L"%s\n", L"file;buf_size;LZ77 bytes needed;LZSS bytes needed;LZ77 compressed size;original size/LZSS compressed size;LZ77 compression rate;LZSS compression rate;LZ77 compression time;LZ77 uncompression time;LZSS compression time;LZSS uncompression time;LZ77 fraction;LZSS fraction");
    //fwprintf(f, L"%s\n", L"file;block_size;entropy;compression time;uncompression time;fraction");
    fwprintf(f, L"%s\n", L"symb_size;entropy");
    for (int file = 0; file < 9; ++file) if (include_file[file])
    {
        d = LoadFromFile(test_files[file]);

        //fwprintf(f, L"\"%s\";;;;;%llu;;;;;;;;\n", test_files[file], dataset_size(d));
        for (int x = X[0]; x <= X[1]; ++x)
            //for (int y : Y0)
        {
            //var = (x<32 ? 16+64*x : (x < 64 ? 256*x-6144 : (x < 131 ? 768*x - 38912 : 61440)));
            wprintf(L"Testing \"%s\" for %s with params x=%d: ", test_files[file], test_for, x);

            //time_point(0);
            //b = TransformBWT(d, var);
            //putchar('B');
            //g = TransformMTF(b);
            //putchar('M');
            //time_point(2);
            //a = DetransformMTF(g);
            //putchar('U');
            //c = DetransformBWT(a);
            //putchar('U');
            //time_point(4);


            time_point(0);
            b = CompressHA(d, false);
            putchar('H');
            time_point(1);
            g = UncompressHA(b);
            putchar('U');
            time_point(2);
            a = CompressHA(d);
            putchar('H');
            time_point(3);
            c = UncompressHA(a);
            putchar('U');
            time_point(4);


            //time_point(0);
            //b = CompressLZ77(d, var);
            //putchar('C');
            //time_point(1);
            //g = UncompressLZ77(b);
            //putchar('U');
            //time_point(2);
            //a = CompressLZSS(d, var);
            //putchar('C');
            //time_point(3);
            //c = UncompressLZSS(a);
            //putchar('U');
            //time_point(4);


            if (*d == *g && *d == *c)
                puts(" success");
            else
            {
                puts(" fail");
                goto EXIT;
            }


            fwprintf(f, L"\"%s\";%llu;%llu;%llu;%llu;%llu;%.4f;%.4f;%llu;%llu;%.15f;%llu;%llu;%.15f\n", test_files[file], dataset_size(d),
                dataset_size(b), b->first.size() - 4 - d->first.size(), dataset_size(a), a->first.size() - 4 - d->first.size(),
                compression_rate(dataset_size(d), dataset_size(b)), compression_rate(dataset_size(d), dataset_size(a)),
                milliseconds(0, 1), milliseconds(1, 2), float(milliseconds(0, 1)) / float(milliseconds(1, 2)),
                milliseconds(2, 3), milliseconds(3, 4), float(milliseconds(2, 3)) / float(milliseconds(3, 4))
            );

            //fwprintf(f, L"%d;%.8f\n", x, Entropy(g, x)            );

            //buf = min_bytes(var);
            //fwprintf(f, L";%u;%u;%u;%llu;%llu;%.4f;%.4f;%llu;%llu;%llu;%llu;%.15f;%.15f\n", var, buf, buf + (var & (0x80 << (buf * 8 - 8)) ? 1:0),
            //    dataset_size(b), dataset_size(a), compression_rate(dataset_size(d), dataset_size(b)), compression_rate(dataset_size(d),dataset_size(a)),
            //    milliseconds(0,1), milliseconds(1,2), milliseconds(2,3), milliseconds(3,4),
            //    float(milliseconds(0, 1))/float(milliseconds(1, 2)), float(milliseconds(2, 3))/float(milliseconds(3, 4))
            //);

            fflush(f);
            delete b;
            delete g;
            delete a;
            delete c;

        }
        delete d;
    }
EXIT:
    fclose(f);

    return -hr;
}
