# MyStockExchangeBoostQTServerClientConsoleApp
My Application of Stock Exchange that can make deals with currencies. IO Console. Will be developed in QT

# Требования
C++17 и выше, CMAke 3.26.4, Boost 1.83.0, QT

# Описание
В репозитории хранятся три четыре папки: Algorithm, MyClient, MyServer, MyclientAppCmake. В первой папке находится исходный код для алгоритма проведения сделки по торгами валютой на бирже. Описание асимптотики и самого алгоритма находится внутри. Там же есть тесты. 
MyClient, MyServer содержат консольные версии приложения. 
MyclientAppCmake – первая сырая версия с графическим интерфейсом QT. Функционал пока основной. Далее будут добавляться все новые модули – работа с базой данных MySql, новый интерфейс и возможности.

# *Ошибки*
Цветовая гамма и расположение кнопок приложения выбрана не лучшим образом. Но это для прототипа. Далее будут использованы текстуры и архитектура по типу других приложений. 
Пока нет проверок на неправильные действия пользователя. Будет исправлено в ближайшее время. 
Архитектура кода также будет изменяться. На данном этапе используется наивная реализация. Многие компоненты и функции нужно оптимизировать.

Примеры работы приложения предоставлены в [видео]([https://youtu.be/pxsS2-DK5ww](https://youtu.be/DyxSNtKIteA) и на картинках внизу. 

*Фото Приложение*
![IMG20240325032203](https://github.com/Evgenicast/MyStockExchangeBoostQTServerClientConsoleApp/assets/107400788/046f2e45-617c-43e2-aab9-8b67f35199a5)
![IMG20240325032342](https://github.com/Evgenicast/MyStockExchangeBoostQTServerClientConsoleApp/assets/107400788/a8907ba4-2e00-44cf-b4cc-2d5065c9e781)
![IMG20240325032352](https://github.com/Evgenicast/MyStockExchangeBoostQTServerClientConsoleApp/assets/107400788/a9fb0066-bd7e-4671-9996-cf224c2b444a)
![IMG20240325032408](https://github.com/Evgenicast/MyStockExchangeBoostQTServerClientConsoleApp/assets/107400788/9022053d-f8f6-4490-850f-49193d01eab0)

*Фото Консольный вариант*

![prog1](https://github.com/Evgenicast/MyStockExchangeBoostQTServerClientConsoleApp/assets/107400788/ab61edc0-8f13-4db3-8495-0bf697606c81)
![prog2](https://github.com/Evgenicast/MyStockExchangeBoostQTServerClientConsoleApp/assets/107400788/16159e16-1f92-4b66-8747-e110f797c148)
