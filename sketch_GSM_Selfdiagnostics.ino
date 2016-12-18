//#include <EEPROM.h>;

unsigned long Resurs = 0;  // Глобальная наработка двигателя, записанная в  EEPROM
unsigned long Resurs1 = 0; // Глобальная наработка во время исполнения программы
unsigned long Resurs2 = 0; // Наработка по окончанию цикла работы двигателя
unsigned long Bounce = 0; // отсечка дребезга контактов
unsigned long Stopengine = 0; // стартовое значение счетчика задержки остановки двигателя
unsigned long Beginresurs = 0; //стартовое значение счетчика ресурса на время цикла работы двигателя
unsigned long Tensioncounter = 0; // 
int Mikrikcounter = 0; // Счетчик срабатываний микрика
int Mikrikstate = 0; // состояние микрика
int Oilstate = 0; // состояние датчика утечки масла
const int Mikrik = 2; // пин микрика
const int Modulestart = 3; // пин програмного старта модуля
const int Startengine = 7; // пин двигателя
const int Powerpin = A6; // Контроль напряжения питания
const int Oil = 5; //пин датчика масла
boolean Stoptime = false; //флаг работы счетчика задержки остановки двигателя
boolean Engine = false; // флаг работающего двигателя
boolean Allert = false; //Флаг критической ошибки и невозможности запуска двигателя 
boolean SMSOil = false;  // Флаг "Отправлена СМС о критическом уровне масла"
boolean SMSAir = false;  // Флаг "Отправлена СМС об обрыве в контуре высокого давления"
boolean SMSResurs1 = false;  // Флаг "Отправлена СМС о необходимости замены масла"
boolean SMSResurs2 = false;  // Флаг "Отправлена СМС о критической необходимости замены масла"
boolean SMSTest = false;  // Флаг "Отправлена тестовая СМС"
boolean Tensionflag = false; // Флаг запуска отсчета времени контроля давления
boolean MikrikOn = false; // Флаг счетчика срабатываний микрика

String currStr = "";
boolean isStringMessage = false;  // Переменная принимает значение True, если текущая строка является сообщением

//___________________ Стартовый цикл _________________________________

void setup()
{
// писк бузера
//программный запуск модуля
//Serial.begin(19200);     // GSM SMS-передача плата SIM900
Serial.begin(9600);      // тестовая Связь по терминалу
//delay(2000);
//NastroykaGSM();           //Функция  Настройки SIM900
//сигнал о готовности

pinMode(Mikrik, INPUT);
//pinMode(Modulestart, OUTPUT);
pinMode(Startengine, OUTPUT);
pinMode(Oil, INPUT);

Resurs = 10000; //cчитать счетчик из EEPROM, присвоено тестовое значение
Resurs1 = Resurs;
digitalWrite(Startengine, HIGH);
//писк бузера
}



void loop()
{
Oilstate = digitalRead(Oil);
Mikrikstate = digitalRead(Mikrik);

//проверяем на критические состояния

// Мониторинг питания для записи счетчика в память при выключении
if (analogRead(Powerpin)< 512) 
  {
   Allert = true;
   Serial.println("Power down!"); // Тестовое сообщение
   //запись значения счетчика ресурса в память 
   // писк бузера
   }

// Проверка датчика масла
else if (Oilstate == HIGH)
{
  Allert = true;
  Serial.println("Low Oil!");
  if (SMSOil = false)
  startOneSMS(); Serial2.println("Alarm!Voda v podpole!"); EndSMS(); 
 // писк бузера
//  отправка сообщения о критическом уровне масла

}
else if (Resurs1 > 1476000)
  {
  Allert = true;
//  сообщение о критической необходимости замены масла
// писк бузера 
  }
else
  {
  Allert = false;
  }
if (Allert == true) //блокировка двигателя при аварии
  {
  digitalWrite(Startengine, HIGH);

  if((Beginresurs != 0) & (Engine == true))
  {
    Resurs2 = (millis() - Beginresurs);
    Resurs1 = (Resurs1 + Resurs2);
    Serial.println("allert Engine stop, write to EEPROM = "); // тестовый вывод наработки с начала работы программы
    Serial.println(Resurs1);
    Engine = false;
  }
  
  }
else if (Mikrikstate == HIGH)//запуск двигателя при замкнутом микрике 
  {
  digitalWrite(Startengine, LOW);
  Stoptime = false;
  if(Engine == false)
    {
      Beginresurs = millis(); //запуск счетчика ресурса в цикле работы двигателя
      Engine = true; //переключение флага работы двигателя
      Mikrikcounter = Mikrikcounter ++;
    }
  } 
else if (Stoptime == false)//запуск задержки остановки двигателя
  {
  Stopengine = millis();

  Stoptime = true;
  }
else if ((millis() - Stopengine) >= 5000)//установка задержки остановки двигателя
  {
  digitalWrite(Startengine, HIGH); 
  if (Engine == true)
    {
    Resurs2 = (millis() - Beginresurs);
    // Resurs3 = int(Resurs2);
     Resurs1 = (Resurs1 + Resurs2);
     Serial.println("Engine off, write to EEPROM = "); //тестовый вывод ресурса наработки со времени старта программы
     Serial.println(Resurs1);
     Engine = false;
     }
   }
// Определение повреждения в системе высокого давления, если микрик сработал три раза за определенное время
if (Mikrikstate == HIGH) // Счетчик срабатываний микрика
  {
    if (MikrikOn == false && (millis()- Bounce) > 500)
     {
        Bounce = millis();
        Mikrikcounter = ++Mikrikcounter;
        Serial.println(Mikrikcounter); //тестовый вывод срабатываний микрика
        MikrikOn = true;
        if (Tensionflag = false)  
         {
            Tensioncounter = millis();
            Tensionflag = true;
         }
     }
  }
else
  {
   MikrikOn = false;
  } 


if ((millis() - Tensioncounter) >= 3000)
  {
   Tensioncounter = millis();
   Tensionflag = false;
   if (Mikrikcounter > 2)
     { 
      Mikrikcounter = 0;
      Serial.println("Tension allert"); //Тестовый отчет о проблеме в контуре высокого давления
      //бузер про обрыв контура давления
      //СМС о том же
     }   
   else 
      {
      Mikrikcounter = 0;
      }
  }

/* Бузера
tone(9, 880, 500);
delay(1000);
tone(9, 1046, 500);
delay(1000);
tone(9, 1174, 500);

*/
}






/* void NastroykaGSM()
{
  Serial.print("AT+CMGF=1\r");         //устанавливает текстовый режим смс-сообщения
    delay(100);
  Serial.print("AT+IFC=1, 1\r");       //устанавливает программный контроль потоком передачи данных
    delay(100);
  Serial.print("AT+CPBS=\"SM\"\r");    //открывает доступ к данным телефонной книги SIM-карты
    delay(100);
  Serial.print("AT+GSMBUSY=1, 1\r");   //запрет всех входящих звонков
    delay(100);
  Serial.print("AT+CNMI=1,2,2,1,0\r"); //включает оповещение о новых сообщениях
    delay(300);
}

void startOneSMS() //__________________Цикл подготовки модуля к отправке СМС-сообщений по первому номеру
{
      Serial.print("AT+CMGF=1\r");
      delay(100);
      Serial.println("AT + CMGS = \"+380632301696\""); // (Номер сервисного центра)
      delay(100);
}      
void startTwoSMS() //__________________Цикл подготовки модуля к отправке СМС-сообщений по второму номеру
{
      Serial.print("AT+CMGF=1\r");
      delay(100);
      Serial.println("AT + CMGS = \"+38000000000\""); // (Номер владельца)
      delay(100);
}    
void EndSMS() //__________________Цикл окончания и отправки СМС-сообщения_______________________
{
   delay(100);
   Serial.println((char)26);                       // Команда отправки СМС
   delay(5000);
}
*/
