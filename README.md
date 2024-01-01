# Arduino Winding Machine

Это продолжение развития прошивки намоточного станка, созданного [Дмитрием Торцевым](https://vk.com/club192215032). Форк основан на версии прошивки [2.1b](https://github.com/apaex/ArduinoWindingMachine/tree/v2.1b), ничего более нового я не нашел. Ссылки на статьи и оригинальные версии прошивок:

* [Намоточный станок на Arduino версия 2.0](https://cxem.net/arduino/arduino245.php)
* [Намоточный станок на Arduino](https://cxem.net/arduino/arduino235.php)

### Список изменений:

* Новый модуль намотки. Теперь податчик перемещается непрерывно, без рывков, все разгоны и торможения делаются плавно, с ускорением, которое можно установить в настройках.
* Новый модуль управления позициями двигателей. Плавный разгон и торможение, настройка шага
* Поддержка многострочных дисплеев. 20х4, 16х2 и т.п.
* Русский язык интерфейса. Прикиньте)
* Сохранение всех настроек в энергонезависимой памяти

#### Eщё по мелочи:

* Сохраняются настройки для 3х трансформаторов по 3 обмотки в каждом. Итого 9 групп параметров. Есть возможность наматывать трансформатор целиком, обмотки намотаюстя последовательно через паузу.
* Схема подавления дребезга больше не нужна. Качественная работа с энкодером, программное подавление дребезга, поддержка разных типов энкодеров.
* Блок глобальных настроек. Сейчас можно отключить паузу между слоями и настроить ускорение при разгоне и торможении
* Скоростное редактирование параметров. Если крутить энкодер зажатым, то параметр будет меняться в 10 раз быстрее
* Шаг намотки поддерживается от 0.005 до 10 мм

## Установка

Скачайте прошивку [вот здесь](https://github.com/apaex/ArduinoWindingMachine/releases/latest).

### Необходимые библиотеки

Перед первой сборкой установите все нужные библиотеки из менеджера библиотек Arduino IDE [[как установить](https://alexgyver.ru/lessons/library-using/)]

* LiquidCrystal 1.0.7 либо LiquidCrystal_I2C 1.1.2, в зависимости от подключения дисплея
* GyverStepper **2.6.4**
* EnсButton **2.0.0**
* AnalogKey 1.1.0

### Настройки в коде

Отредактируйте файл [config.h](https://github.com/apaex/ArduinoWindingMachine/blob/main/Arduino_winding_machine/config.h), который лежит в каталоге прошивки, если вы используете железо, отличающееся от того, которое в статье. Например, размер своего дисплея.

* Обратите внимание на тип энкодера (ENCODER_TYPE). Если вы используете толстый энкодер, то он будет "проскакивать" и нужно в файле изменить его тип 
* Проверьте шаг резьбы вала намотчика (THREAD_PITCH), теперь он в мкм

## Траблы для тех, кто только построил станок

* **Ничего не крутится и стоит на паузе** - не нажата педаль, нажмите замыканием контакта шилда Z- на землю
* **Темно и ничего не видно** - подключите выводы дисплея А и К через резистор к напряжению подсветки
* **Светло и ничего не видно** - проверьте подключение вывода дисплея RW к земле или к плате расширения портов
* **Видно только черные прямоугольники** - скорее всего иначе подключен вывод RS, установите правильный пин в прошивке. Или отрегулируйте потенциал на контакте V0 дисплея резистором на плате расширения портов
* **Мусор на экране** - проверьте соответствие способа подключения дисплея настройкам прошивки
* **Энкодер пропускает каждый второй щелчок** - измените тип энкодера в настройках прошивки
* **Меню само хаотично нажимается и творит дичь** - подтяните 10к резистором SW контакт энкодера к питанию

[Скачать](https://github.com/apaex/ArduinoWindingMachine/releases/latest)
