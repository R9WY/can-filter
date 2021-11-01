# can-filter
Test application for filter data CANopen protocol

Требуется написать программу для фильтрации и перенаправления на UDP-сервер потока данных.

1. Программа будет исполняться под *nix-платформой, на архитектурах x86 и ARM, на 32-битных процессорах.
2. Программа будет запускаться другим приложением, с правами суперпользователя.
3. Программа может работать заранее неизвестное, но потенциально неограниченное время.
4. Должна быть возможность штатного завершения работы программы в любой момент.
5. Входной поток данных представляет собой выхлоп программы candump на нашей реальной CAN-шине. Представительный пример выхлопа с необходимыми комментариями приведён в файле dump.txt.
6. Программа должна анализировать поток данных построчно, и пересылать на определённый UDP-сервер те строки, которые принадлежат устройствам с определёнными canID (он же nodeID). Строка при совпадении условия должна пересылаться полностью и без искажений. Пропуск подходящих под условие строк недопустим. Ложные срабатывания также недопустимы.
7. Язык разработки - предпочтительно C++11, но не старше C++14. Альтернативу обосновать.
8. Готовое решение представить ссылкой на репозитарий. Проверка будет выполняться на нашей стороне на нашем железе нашим сотрудником.
9. Жёстких ограничений по использованию ОЗУ и по размеру исполняемого файла нет, но следует учитывать, что программа будет запускаться на встраиваемом устройстве. Следует минимизировать запись на диск.
10. В дальнейшем программа будет использоваться, сопровождаться и дорабатываться нашими сотрудниками, не обязательно автором программы.

==============
Решение

1. Для компиляции x86_32 используется команда make
   sudo make install устанавливает в /usr/local/bin
   Компиляция под ARM:  arm-linux-gnueabi-g++ main.cpp -o can-filter-arm -static
2. Входным потоком является поток stdin, который может быть перенаправлен из другого приложения или файла
3. Написан тест candump-emu для эиуляции вывода, проверена утечка памяти и стабильность работы
    test-udp-server.c - тестовый UDP-сервер для отладки
4. В программе выполняется перехват сигналов SIGINT, SIGTERM, выход осуществляется после завершения передачи на сервер
7. Программа разработана с использованием синтаксиса языка С++11
8. Репозитарий находится https://github.com/R9WY/can-filter.git
    В ветке fast - упрощенный вариант, без отслеживания сигналов завершения программы.
9. Запись на диск не производится

