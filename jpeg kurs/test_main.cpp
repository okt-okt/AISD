#define _MAIN_FILE_
#include <chrono>
#include "image.h"
#include <sstream>
#include <string>




int main()
{
    setlocale(LC_ALL, "Russian");
    std::wstring out_file;
    image* d {}, * b {}, * g {};
    uint32_t buf;
    std::vector<std::wstring> custom_files;
    FILE* f {};
    const wchar_t* stat_file, * uncompr_str = L"jpeg\\Uncompression", * compr_str = L"jpeg\\Compression", * stat_str = L"jpeg\\Statistics";
    int hr = 0, var;
    wchar_t line[200];
    wchar_t s[1000]; int si = 0, ddd;

    auto compression_rate = [](size_t orig, size_t compr)
        {
            return 100.f * (orig > compr ? float(orig - compr) : -float(compr - orig)) / float(orig);
        };
    auto get_name_of_result = [&](const wchar_t* result_of, const wchar_t* orig_name, uint32_t quality, const wchar_t* additional, const wchar_t* result_ext)->wchar_t*
        {
            si = wcslen(result_of);
            memcpy(s, result_of, si * sizeof(wchar_t));
            memcpy(s + si, L" result for ", 12 * sizeof(wchar_t));
            si += 12;
            for (; *orig_name; ++orig_name, ++si)
                if ((s[si] = uppercase(*orig_name)) == *orig_name && !((L'A' <= *orig_name && *orig_name <= L'Z') || (L'0' <= *orig_name && *orig_name <= L'9')))
                    s[si] = L' ';
            if (quality)
            {
                memcpy(s + si, L" (q=", 4 * sizeof(wchar_t));
                si += 4;
                for (ddd = (quality > 99 ? 100 : (quality > 9 ? 10 : 1)); ddd; ddd /= 10)
                    s[si++] = L'0' + (quality / ddd) % 10;
                if (additional)
                {
                    s[si++] = L' ';
                    memcpy(s + si, additional, wcslen(additional) * sizeof(wchar_t));
                    si += wcslen(additional);
                }
                s[si++] = L')';
            }
            else if (additional)
            {
                memcpy(s + si, L" (", 2 * sizeof(wchar_t));
                si += 2;
                memcpy(s + si, additional, wcslen(additional) * sizeof(wchar_t));
                si += wcslen(additional);
                s[si++] = L')';
            }
            if (result_ext[0] != L'.')
                s[si++] = L'.';
            memcpy(s + si, result_ext, (wcslen(result_ext) + 1) * sizeof(wchar_t));
            return s;
        };
#define time_point(i) times[i] = std::chrono::high_resolution_clock::now()
#define milliseconds(i,j) std::chrono::duration_cast<std::chrono::milliseconds>(times[j]-times[i]).count()
#define file_size(q) (q->second.size() + (q->first[0] == TYPE_JPEG ? reinterpret_cast<INTERNAL_JPEG_HEADER*>(q->first.data() + 4)->JPEG_meta_size : (q->first[0] == TYPE_IncompatibleJPEG ? 0 : q->first.size())))

    const wchar_t* test_files[] = { L"Lena.jpg", L"Lena.AISD.RAW", L"Lena-grey.jpg", L"Lena-grey.AISD.RAW", L"borzoi-breed.jpg", L"borzoi-breed.AISD.RAW", L"borzoi-breed-grey.jpg", L"borzoi-breed-grey.AISD.RAW", L"borzoi-breed-bw.AISD.RAW", L"borzoi-breed-dither.AISD.RAW"};

    bool include_file[sizeof(test_files) / sizeof(test_files[0])] {};


    if (true)
    {
    DEFINE_TEST_FILES:
        puts("\nChoose of standard test files:");
        for (int file = 1; file <= (sizeof(test_files) / sizeof(test_files[0])); ++file)
            wprintf(L" %c - %s\n", (file < 10 ? L'0' + file : L'A' + file - 10), test_files[file - 1]);
        printf("Files to include in testing or 0 to add custom file: ");
        wscanf_s(L"%s", line, 200);
        if (*line == L'0')
        {
        ADD_CUSTOM_FILE:
            puts("Input custom test file name (no spaces!): ");
            wscanf_s(L"%s", line, 200);
            custom_files.push_back(line);
            puts("Custom test file was registered!\nChoose the option: 1 - start testing, 2 - add custom file, 3 - choose from standard files");
            wscanf_s(L"%s", line, 200);
            for (stat_file = line; (*stat_file && *stat_file == L' '); ++stat_file);
            switch (*stat_file)
            {
                case L'\0': case L'1': goto START_TESTING;
                case L'2': goto ADD_CUSTOM_FILE;
                case L'3': default: goto DEFINE_TEST_FILES;
            }
        }
        for (int i = 0; i < 200 && line[i]; ++i)
            if (L'1' <= line[i] && line[i] <= L'9')
                include_file[line[i] - L'0' - 1] = true;
            else if (L'A' <= uppercase(line[i]) && uppercase(line[i]) <= (sizeof(test_files) / sizeof(test_files[0])-10+L'A'))
                include_file[uppercase(line[i]) - 'A' + 10 - 1] = true;
    START_TESTING:;
    }

#define SafeDelete(l) if(l){delete l; l=NULL;}
    if (true)
    {
        std::chrono::high_resolution_clock::time_point times[10];

        auto test_program = [&](const wchar_t* filename)
            {
                for (uint8_t DC1 = 0; DC1 < 2; ++DC1) for (uint8_t DC2 = 0; DC2 < 2; ++DC2) for (uint8_t AC1 = 0; AC1 < 2; ++AC1)for (uint8_t AC2 = 0; AC2 < 2; ++AC2)
                {
                    uint8_t HTables[3][2] { { 0, 0 }, { DC1, AC1 }, { DC2, AC2 } };

                    for (wchar_t dir = 0, *caret = line; dir < 2; ++dir)
                    {
                        for (uint8_t n = 0; n < 3; ++n)
                            *(caret++) = '0' + HTables[n][dir];
                        *(caret++) = dir ? 0 : L' ';
                    }

                    wprintf(L"\n\"%s\" (%s): ", filename, line);
                    d = LoadFromFile(filename);
                    if (d)
                        printf("loaded, testing compression:");
                    else
                        puts("ERROR loading file");

                    if (_wfopen_s(&f, get_name_of_result(stat_str, filename, 0, line, L"csv"), L"wt"))
                        return -100;

                    switch (d && d->first.size() ? d->first[0] : 0)
                    {
                        case TYPE_ImageColorful:
                        case TYPE_ImageColorfulY:
                        case TYPE_ImageGrayscale:
                        case TYPE_ImageBW:
                        {
                            fwprintf(f, L"uc;q;jpegc;jpeguc;jpegct;jpeguct;cr;jpegcr;jpegcr2\n");
                            uint32_t q_start = 1, q_end = 100, q_step = 1, save_each_below = 0;
                            for (uint32_t q = q_start; q <= q_end; q += (q == q_end ? 1 : min(q_end - q, q_step)))
                            {
                                if ((q == q_start) || (q == q_end && q_end % 5 != 0) || (q - 1) % 5 == 0)
                                    printf("\n  quality %3u-%3u: ", q, uint32_t(min(5 * ((q - 1) / 5 + 1), q_end)));

                                time_point(0);
                                b = CompressJPEG(d, q, HTables);
                                time_point(1);
                                putchar(b ? 'C' : 'E');
                                time_point(2);
                                g = UncompressJPEG(b);
                                time_point(3);
                                putchar(g ? 'U' : 'E');

                                if (b && g)
                                {
                                    if (q == q_start || q == q_end || q % 5 == 0 || q <= save_each_below)
                                        SaveToFile(b, get_name_of_result(compr_str, filename, q, line, L".jpg")),
                                        SaveToFile(g, get_name_of_result(uncompr_str, filename, q, line, L".aisd.raw"));

                                    fwprintf(f, L"%llu;%u;%llu;%llu;%llu;%llu;%.8f;%.4f;%.4f\n",
                                        file_size(d), q, file_size(b), file_size(g), milliseconds(0, 1), milliseconds(2, 3),
                                        float(file_size(d)) / float(file_size(b)),
                                        compression_rate(file_size(d), file_size(b)), compression_rate(file_size(g), file_size(b)));
                                    fflush(f);
                                }
                                SafeDelete(b); SafeDelete(g);
                            }
                        }
                        break;

                        case TYPE_JPEG:
                        {
                            time_point(0);
                            b = UncompressJPEG(d);
                            time_point(1);
                            putchar('U');
                            SaveToFile(b, get_name_of_result(uncompr_str, filename, 0, NULL, L".aisd.raw")),
                                fwprintf(f, L"c;q;uc;jpegc;jpeguct;jpegct;cr;crq;origcr;jpegcr\n");
                            uint32_t q_start = 1, q_end = 100, q_step = 1, save_each_below = 0;
                            uint8_t HTables[3][2] { { 0, 0 }, { 1, 1 }, { 1, 1 } };
                            for (uint32_t q = q_start; q <= q_end; q += (q == q_end ? 1 : min(q_end - q, q_step)))
                            {
                                if ((q == q_start) || (q == q_end && q_end % 10 != 0) || (q - 1) % 10 == 0)
                                    printf("\n  quality %3u-%3u: ", q, uint32_t(min(10 * ((q - 1) / 10 + 1), q_end)));

                                time_point(2);
                                g = CompressJPEG(b, q, HTables);
                                time_point(3);
                                putchar(g ? 'C' : 'E');

                                if (g)
                                {
                                    if (q == q_start || q == q_end || q % 5 == 0 || q <= save_each_below)
                                        SaveToFile(g, get_name_of_result(compr_str, filename, q, line, L".jpg"));

                                    fwprintf(f, L"%llu;%u;%llu;%llu;%llu;%llu;%.8f;%.8f;%.4f;%.4f\n",
                                        file_size(d), q, file_size(b), file_size(g), milliseconds(0, 1), milliseconds(2, 3),
                                        float(file_size(b)) / float(file_size(d)), float(file_size(b)) / float(file_size(g)),
                                        compression_rate(file_size(b), file_size(d)), compression_rate(file_size(b), file_size(g)));
                                    fflush(f);
                                }
                                SafeDelete(g);
                            }
                        }
                        break;

                        case TYPE_IncompatibleJPEG:
                            puts("loaded, incompatible JPEG.\n");
                            break;

                        default:
                            puts("could not load file or file is incompatible.\n");
                    }

                    if (f)
                        fclose(f);
                    f = NULL;
                    SafeDelete(d); SafeDelete(b); SafeDelete(g);
                }
                return 0;
            };

        for (int file = 0; file < (sizeof(test_files) / sizeof(test_files[0])); ++file) if (include_file[file]) if (hr = test_program(test_files[file])) return hr;
        for (auto& i : custom_files) if (hr = test_program(i.c_str())) return hr;

    FAIL:
        if (f)
            fclose(f);
        return -hr;
    }

EXIT:
    fclose(f);

    return -hr;
}
