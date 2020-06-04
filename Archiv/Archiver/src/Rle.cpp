#include "Rle.h"
#include <map>
#include <string>
#include <QMessageBox>
#include <fstream>
#include <vector>

using namespace std;

void CompressorRLE::compressRLE(QString &compressFileName, QString &rleFileName)
{
    //создаём объекты для чтения и записи
    ifstream fileInput(compressFileName.toStdString());
    ofstream fileCompressed(rleFileName.toStdString());
    char sym1;//первый символ(байт) в файле

    while(fileInput.good())
    {
        int count=1;//кол-во совпавших символов
        fileInput.get(sym1);//считываем первый символ из файла fileInput в sym1
        while (sym1 == fileInput.peek() && count < 9)//пока первый символ равен следующему за ним и кол-во меньше 9 - повторяем
        {
            //увеличиваем кол-во совпадений и считываем второй символ в sym1
            count++;
            fileInput.get(sym1);
        }
        //записываем результат в файл для записи
        fileCompressed << sym1 << count;
    }
    //закрываем оба файла
    fileInput.close();
    fileCompressed.close();
}

void CompressorRLE::decompressRLE(QString &compressedRLEfileName, QString &decompressedFileName)
{
    //создаём объекты для чтения и записи переданные в параметрах
    ifstream fileToDescompress(compressedRLEfileName.toStdString());
    ofstream fileDecompressed(decompressedFileName.toStdString());
    char ch1, ch2;//первый(сам символ) и второй(кол-во символов, т.е число) символ в файле
    fileToDescompress.get(ch1);//считываем перввый символ
    while(fileToDescompress.good())//проверяем не дошли ли мы до конца файла
    {
        fileToDescompress.get(ch2);//считываем второй
        for (int i = 0; i < ch2 - '0'; i++){ //пока не запишем все повторяющиеся символы не выйдем
            fileDecompressed << ch1;
        }
        fileToDescompress.get(ch1);//считываем следующий символ
    }
    //закрываем оба файла
    fileToDescompress.close();
    fileDecompressed.close();
}
