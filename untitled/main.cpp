#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <cstdint>
#include <math.h>

using namespace std;

uint16_t Encode_ham( char ch, int bits[] );
char Decode_ham( uint16_t x );

//--------------------------------------------------------------------
// проверка, что файл с заданным именем уже существует
//--------------------------------------------------------------------
bool Is_file_exist ( const string name_file )
{
    ifstream in( name_file );
    if ( in.is_open() )
    {
        in.close();
        return true;
    }

    return false;
}
//--------------------------------------------------------------------
// узнать размер заданного файла в байтах
//--------------------------------------------------------------------
int Get_size_file_in_bytes( const string name_file )
{
    ifstream in;
    int size;

    in.open( name_file, ios::in | ios::binary ); // бинари нужен чтобы в файле все читалось и сохранялось ровно так, как мы записали, типо / а не //
    in.seekg( 0, ios::end );//идем в конец файла
    size = in.tellg();// узнаем позицию курсора(и количество байт, раз символ весит по 1)
    in.close();

    return size;
}
//--------------------------------------------------------------------
// создание нового архива ( команда "c" )
// если архив с таким именем уже существует, то он не создается
//--------------------------------------------------------------------
void Create_empty_archive( string name_file )
{
    const string EXTENSION = ".haf";
    name_file += EXTENSION;

    if ( Is_file_exist( name_file ) ) //  name file это имя архива, здесь уже с расширением
    {
        cout << "Архив с именем \"" << name_file << "\" уже существует на жестком диске. Создать архив с таким же именем невозможно." << endl << endl;
    }
    else
    {
        ofstream out( name_file );// создает архив
        out.close();
        cout << "Новый пустой архив с именем \"" << name_file << "\" успешно создан на жестком диске." << endl << endl;
    }
}
//--------------------------------------------------------------------
// добавление нового файла в архив ( команда "a" )
// вызов: название_программы a имя_архива_без_расширения полное_имя_файла
// если на диске нет файла-архива с заданным именем, то файл не добавляется
//--------------------------------------------------------------------
void Append_file_to_archive( string name_archive, string name_add_file )
{
    const string EXTENSION = ".haf";
    name_archive += EXTENSION;

    if ( ! Is_file_exist( name_archive ) )// сворачиваеся если архива нет
    {
        cout << "Архив с именем \"" << name_archive << "\" не существует на жестком диске. Добавить в него файл физически невозможно." << endl << endl;
        return;
    }

    fstream out;
    out.open( name_archive, ios::in | ios::out | ios::binary ); // здесь нужны оба ios::in | ios::out тк их нет по умолчанию(например у ifstream ios::in, есть по умолчанию и его писать не надо)

    int files_count_in_archive = 1;
    int size_archive_in_bytes = Get_size_file_in_bytes( name_archive );
    if ( size_archive_in_bytes > 0 )
    {
        out.read( ( char*) &files_count_in_archive, sizeof( int ) ); // тот же std::cin но который коректней работает с файла в ios::binary, второй аргумент- ограничение максимальной длинны, в первый сохраняем значение
        files_count_in_archive++;
        out.seekg( 0, ios::beg );// теперь курсор стоит в начале файла
    }
    out.write( ( char* ) &files_count_in_archive, sizeof( int ) );		// количество файлов в архиве // как read, но тот же cout

    // встаем в конец архива
    out.seekg( 0, ios::end );// курсор стоит в конце файла
    ifstream in;
    in.open( name_add_file, ios::in | ios::binary );
    int len_name_add_file = name_add_file.length();
    out.write( ( char* ) &len_name_add_file, sizeof( int ) );			// размер имени файла
    // записываем название файла в архив
    for ( int i = 0; i < len_name_add_file; i++ )
    {
        out.put( name_add_file[ i ] );// посимвольно добаляем название файла в архив
    }

    // узнаем, сколько байт в файле, который добавляется в архив
    int size_add_file = Get_size_file_in_bytes( name_add_file );
    out.write( ( char* ) &size_add_file, sizeof( int ) );				// размера данных файла в байтах

    // побайтово пишем данные из файла в архив
    char current_byte;
    while ( in )
    {
        in.get( current_byte );
        if ( in )
        {
            int bits[ 16 ] = { 0 };
            uint16_t ch = Encode_ham( current_byte, bits );
            // out.put( current_byte );
            out.write( ( char* ) &ch, sizeof ( uint16_t ) );// записываем хеминговую версию битов в архив
        }
    }

    cout << endl << "Файл с именем \"" << name_add_file << "\" успешно добавлен в заданный архив." << endl;
    // закрываем все потоки
    in.close();
    out.close();
}
//--------------------------------------------------------------------
// вывод названий всех файлов, которые находятся в архиве
// вызов: название_программы l имя_архива_без_расширения
//--------------------------------------------------------------------
void Print_list_name_files_in_archive( string name_archive )
{
    const string EXTENSION = ".haf";
    name_archive += EXTENSION;

    if ( ! Is_file_exist( name_archive ) )
    {
        cout << "Архив с именем \"" << name_archive << "\" не существует на жестком диске. Получить список файлов физически невозможно." << endl << endl;
        return;
    }

    ifstream in;
    in.open( name_archive, ios::in | ios::binary ); //читаем архив

    int files_count_in_archive = 1;
    int size_archive_in_bytes = Get_size_file_in_bytes( name_archive );
    if ( size_archive_in_bytes > 0 )
    {
        in.read( ( char*) &files_count_in_archive, sizeof( int ) );
        cout << "Список имен файлов в заданном архиве: " << endl;
        for ( int i = 1; i <= files_count_in_archive; i++ )
        {
            int len_name_file;
            string name_file = "";
            in.read( ( char* ) &len_name_file, sizeof ( int ) );
            char current;

            for ( int j = 0; j < len_name_file; j++ )
            {
                in.get( current );
                name_file += current;
            }
            cout << "\t - " << name_file << endl;

            // узнаем кол-во байт в текущем файле и пропускаем их, чтобы
            // перейти к обработке след. файла
            int size_data;
            in.read( ( char* ) &size_data, sizeof ( int ) );
            char* buf = new char[ size_data * 2 + 1 ];
            in.read( buf, size_data * 2 );
            delete[] buf;
        }
        cout << endl << endl;
    }
    else
    {
        cout << "Архив с именем \"" << name_archive << "\" существует на жестком диске, но он пустой, т е не содержит ни одного файла." << endl << endl;
    }
}
//--------------------------------------------------------------------
// извлечение всех файлов из заданного архива
// вызов: название_программы x имя_архива_без_расширения
//--------------------------------------------------------------------
void Extract_all_files_from_archive( string name_archive )
{
    const string EXTENSION = ".haf";
    name_archive += EXTENSION;

    if ( ! Is_file_exist( name_archive ) )
    {
        cout << "Архив с именем \"" << name_archive << "\" не существует на жестком диске. Извлечь список файлов физически невозможно." << endl << endl;
        return;
    }

    ifstream in;
    in.open( name_archive, ios::in | ios::binary );

    int files_count_in_archive;
    int size_archive_in_bytes = Get_size_file_in_bytes( name_archive );
    if ( size_archive_in_bytes > 0 )
    {
        in.read( ( char*) &files_count_in_archive, sizeof( int ) );
        for ( int i = 1; i <= files_count_in_archive; i++ )
        {
            ofstream out;
            int len_name_file;
            string name_file = "";
            in.read( ( char* ) &len_name_file, sizeof ( int ) );
            char current;

            // считываем название текущего извлекаемого файла из архива
            for ( int j = 0; j < len_name_file; j++ )
            {
                in.get( current );
                name_file += current;
            }
            out.open( name_file, ios::out | ios::binary );

            // узнаем кол-во байт в текущем файле, чтобы
            // их переписать в выходной файл
            int size_data;
            in.read( ( char* ) &size_data, sizeof ( int ) );

            for ( int k = 0; k < size_data; k++ )
            {
                uint16_t n;
                in.read( ( char* ) &n, sizeof ( uint16_t ) );
                char ch = Decode_ham( n );
                out.put( ch );
            }

            //char* buf = new char[ size_data + 1 ];
            //in.read( buf, size_data );
            //out.write( buf, size_data );
            //delete[] buf;

            out.close();
        }
        in.close();
        cout << "Из архива с именем \"" << name_archive << "\" были успешно извлечены все файлы." << endl << endl;
    }
    else
    {
        cout << "Архив с именем \"" << name_archive << "\" существует на жестком диске, но он пустой, т е не содержит ни одного файла." << endl << endl;
    }
}
//--------------------------------------------------------------------
// проверка, что удаляемый файл есть в архиве
//--------------------------------------------------------------------
int Get_number_deleted_file( string name_archive, string delete_name_file )
{
    int result = -1;
    const string EXTENSION = ".haf";
    name_archive += EXTENSION;

    if ( ! Is_file_exist( name_archive ) )
    {
        cout << "Архив с именем \"" << name_archive << "\" не существует на жестком диске. Удаление физически невозможно." << endl << endl;
        return result;
    }

    ifstream in;
    in.open( name_archive, ios::in | ios::binary );

    int files_count_in_archive = 1;
    int size_archive_in_bytes = Get_size_file_in_bytes( name_archive );
    if ( size_archive_in_bytes > 0 )
    {
        in.read( ( char*) &files_count_in_archive, sizeof( int ) );
        for ( int i = 1; i <= files_count_in_archive; i++ )
        {
            int len_name_file;
            string name_file = "";
            in.read( ( char* ) &len_name_file, sizeof ( int ) );
            char current;

            for ( int j = 0; j < len_name_file; j++ )
            {
                in.get( current );
                name_file += current;
            }

            if ( name_file == delete_name_file )
            {
                result = i;
                return result;
            }

            // узнаем кол-во байт в текущем файле и пропускаем их, чтобы
            // перейти к обработке след. файла
            int size_data;
            in.read( ( char* ) &size_data, sizeof ( int ) );
            char* buf = new char[ size_data * 2 + 1 ];
            in.read( buf, size_data * 2 );
            delete[] buf;
        }
        return result;
    }
    else
    {
        cout << "Архив с именем \"" << name_archive << "\" существует на жестком диске, но он пустой, т е не содержит ни одного файла." << endl << endl;
        return result;
    }
}
//--------------------------------------------------------------------
// удаление заданного файла из заданного архива
// вызов: название_программы d имя_архива_без_расширения имя_файла_с_расширением
//--------------------------------------------------------------------
void Delete_file_from_archive( string name_archive, string name_file, int number )
{
    const string EXTENSION = ".haf";
    name_archive += EXTENSION;

    ifstream in;
    in.open( name_archive, ios::in | ios::binary );

    int files_count_in_archive;
    in.read( ( char*) &files_count_in_archive, sizeof( int ) );

    // если удаляется единственный файл из архива
    if ( files_count_in_archive == 1 )
    {
        in.close();
        remove( name_archive.c_str() );
        ofstream out( name_archive );
        out.close();

        return;
    }

    ofstream out;
    string tmp_name_file = "tmp1111.haf";
    out.open( tmp_name_file, ios::out | ios::binary );

    int new_count_files = files_count_in_archive - 1;
    out.write( ( char* ) &new_count_files, sizeof( int ) );

    for ( int i = 1; i <= files_count_in_archive; i++ )
    {
        int len_name_file;
        string name_file = "";
        in.read( ( char* ) &len_name_file, sizeof ( int ) );

        if ( i != number )
        {
            out.write( ( char* ) &len_name_file, sizeof( int ) );
        }

        char current;

        for ( int j = 0; j < len_name_file; j++ )
        {
            in.get( current );
            if ( i != number )
            {
                out.put( current );
            }
            name_file += current;
        }

        // узнаем кол-во байт в текущем файле и пропускаем их, чтобы
        // перейти к обработке след. файла
        int size_data;
        in.read( ( char* ) &size_data, sizeof ( int ) );
        if ( i != number )
        {
            out.write( ( char* ) &size_data, sizeof( int ) );
        }
        char* buf = new char[ size_data * 2 + 1 ];
        in.read( buf, size_data * 2 );
        if ( i != number )
        {
            out.write( buf, size_data * 2 );
        }
        delete[] buf;
    }
    in.close();
    remove( name_archive.c_str() );
    out.close();
    rename( tmp_name_file.c_str(), name_archive.c_str() );
}
//--------------------------------------------------------------------
// меню программы
//--------------------------------------------------------------------
int Menu( void )
{
    int select;

    do
    {
        system( "CLS" );
        cout << "1 - Создать пустой ( новый ) архив" << endl;
        cout << "2 - Добавить в архив новый файл" << endl;
        cout << "3 - Вывести список файлов, упакованных в архив" << endl;
        cout << "4 - Извлечь все файлы из заданного архива" << endl;
        cout << "5 - Удалить файл из архива" << endl;
        cout << "6 - Выход из программы" << endl;
        cout << "\tВЫБОР: ";
        cin >> select;
    }
    while ( ( select < 1 ) || ( select > 6 ) );

    fflush( stdin );
    return select;
}

uint16_t From_bits_to_ham_code( int bits[] )
{
    uint16_t n = 0;

    for ( int i = 0; i < 16; i++ )
    {
        if ( bits[ i ] != 0 )
        {
            n += pow( 2.0, 16 - i - 1 );
        }
    }

    return n;
}

int Calculate_parity( int arr[], int n, int m );
void Add_control_bits( int input[], int n, int bits[] );
//--------------------------------------------------------------------
// перевод символа в набор из 8ми бит
//--------------------------------------------------------------------
uint16_t Encode_ham( char ch, int bits[] )
{
    uint8_t n = ( uint8_t )ch;
    int input[ 8 ] = { 0 };

    int k = 7;
    while ( n > 0 )
    {
        input[ k-- ] = n % 2;
        n /= 2;
    }

    Add_control_bits( input, 8, bits );

    uint16_t ham = From_bits_to_ham_code( bits );
    return ham;
}
//--------------------------------------------------------------------
// добавление контрольных битов в массив из 8 бит ( символ )
//--------------------------------------------------------------------
void Add_control_bits( int input[], int n, int bits[] )
{
    int m, flag, parity[ 10 ], i, j, k;

    for ( i = 0, m = 0; i < n; i++ )
    {
        if ( ( int ) pow( 2, i * 1.0 ) <= n )
            m++;
        else
            break;
    }
    m--;

    for ( i = 0, k = 0; i < n; i++ )
    {
        for ( j = 0, flag = 0; j <= m; j++ )
            if ( i + 1 == pow( 2, j * 1.0 ) )
            {
                flag = 1;
                bits[ i ] = 7;
                n++;
                break;
            }
        if ( flag == 0 )
        {
            bits[ i ] = input[k];
            k++;
        }
    }

    for ( i = 0; i <= m; i++ )
    {
        bits[ ( int )pow( 2, i * 1.0 ) - 1 ] = parity[ i ] = Calculate_parity( bits, n, ( int )pow( 2, i * 1.0 ) );
    }

    //for ( i = 0; i < n; i++ )
    //	cout << bits[ i ];
}
//--------------------------------------------------------------------
int Calculate_parity( int arr[], int n, int m )
{
    int i, sum, c;
    for ( i = m - 1, c = 0, sum = 0; i < n; i++ )
    {
        if ( i == m - 1 )
        {
            sum = 0;
            c++;
        }
        else
        {
            sum += arr[ i ];
            c++;
        }

        if ( c == m )
        {
            c = 0;
            i += m;
        }
    }

    if ( sum % 2 == 0 )
        return 0;
    else
        return 1;
}
//--------------------------------------------------------------------
// декодирование кода Хэмминга в символ
//--------------------------------------------------------------------
char Decode_ham( uint16_t x )
{
    int bits[ 16 ] = { 0 };
    int k = 16;

    while ( x > 0 )
    {
        bits[ --k ] = x % 2;
        x /= 2;
    }

    int i, j, m = 3, n = 12;
    int decode_bits[ 8 ] = { 0 };
    int pos = 0;
    for( i = 0, j = 0; i < n; i++ )
        if ( ( i != ( ( ( int )pow( 2, j * 1.0 ) ) - 1 ) ) || ( j > m ) )
            decode_bits[ pos++ ] = bits[ i ];
        else
            j++;

    char ch = 0;
    for ( i = 0; i < 8; i++ )
    {
        if ( decode_bits[ i ] != 0 )
        {
            ch += pow( 2.0, 8 - i - 1 );
        }
    }

    return ch;
}
//--------------------------------------------------------------------
// главная функция программы ( точка входа )
//--------------------------------------------------------------------
int main( void )
{
    setlocale( LC_ALL, "Russian" );

    int select;
    do
    {
        select = Menu();
        cout << endl;
        switch ( select )
        {
            case 1:
            {
                string name_archive;
                cout << "Введите имя архива БЕЗ расширения: ";
                getline( cin, name_archive );

                Create_empty_archive( name_archive );
                break;
            }

            case 2:
            {
                string name_archive;
                cout << "Введите имя архива БЕЗ расширения: ";
                getline( cin, name_archive );

                string name_file;
                cout << "Введите имя файла, который добавляется в архив, С раширением: ";
                getline( cin, name_file );

                Append_file_to_archive( name_archive, name_file );
                break;
            }

            case 3:
            {
                string name_archive;
                cout << "Введите имя архива БЕЗ расширения: ";
                getline( cin, name_archive );

                Print_list_name_files_in_archive( name_archive );
                break;
            }

            case 4:
            {
                string name_archive;
                cout << "Введите имя архива БЕЗ расширения: ";
                getline( cin, name_archive );

                Extract_all_files_from_archive( name_archive );
                break;
            }

            case 5:
            {
                string name_archive;
                cout << "Введите имя архива БЕЗ расширения: ";
                getline( cin, name_archive );

                string name_file;
                cout << "Введите имя файла, который планируется удалить из архива, С раширением: ";
                getline( cin, name_file );

                int number = Get_number_deleted_file( name_archive, name_file );
                if ( number > 0 )
                {
                    Delete_file_from_archive( name_archive, name_file, number );
                    cout << "Заданный файл успешно удален из архива." << endl << endl;
                }
                else
                {
                    cout << "Заданный файл отсутствует в архиве! Удаление физически невозможно." << endl << endl;
                }
                break;
            }
        }
        if ( select != 6 )
        {
            system( "pause" );
        }
    }
    while ( select != 6 );

    return EXIT_SUCCESS;
}
//--------------------------------------------------------------------
