//#include <EEPROM.h>;

unsigned long resGlobal = 0;  // глобальная наработка двигателя, записанная в  EEPROM
unsigned long resOper = 0; // наработка во время исполнения программы
unsigned long resStop = 0; // Наработка по окончанию цикла работы двигателя
unsigned long bounce = 0; // отсечка дребезга контактов
unsigned long stopengine = 0; // стартовое значение счетчика задержки остановки двигателя
unsigned long beginRes = 0; //стартовое значение счетчика ресурса на время цикла работы двигателя
unsigned long tensioncounter = 0; // 
int mikrikcounter = 0; // Счетчик срабатываний микрика
int mikrikstate = 0; // состояние микрика
int oilstate = 0; // состояние датчика утечки масла
const int mikrik = 2; // пин микрика
const int modulestart = 3; // пин програмного старта модуля
const int startengine = 7; // пин двигателя
const int powerpin = A6; // Контроль напряжения питания
const int oil = 5; //пин датчика масла
boolean stoptime = false; //флаг работы счетчика задержки остановки двигателя
boolean engine = false; // флаг работающего двигателя
boolean allert = false; //Флаг критической ошибки и невозможности запуска двигателя 
boolean smsoil = false;  // Флаг "Отправлена СМС о критическом уровне масла"
boolean smsAir = false;  // Флаг "Отправлена СМС об обрыве в контуре высокого давления"
boolean smsRes1 = false;  // Флаг "Отправлена СМС о необходимости замены масла"
boolean smsRes2 = false;  // Флаг "Отправлена СМС о критической необходимости замены масла"
//boolean smsTest = false;  // Флаг "Отправлена тестовая СМС"
boolean tensionflag = false; // Флаг запуска отсчета времени контроля давления
boolean mikrikOn = false; // Флаг счетчика срабатываний микрика
//boolean isStringMessage = false;  // Переменная принимает значение True, если текущая строка является сообщением
//String currStr = "";

//___________________ Стартовый цикл _________________________________

void setup()
{
// писк бузера
//программный запуск модуля
//Serial.begin(19200);     // GSM SMS-передача плата SIM900
Serial.begin(9600);      // тестовая Связь по терминалу
//delay(2000);
//GSMSetup();           //Функция  Настройки SIM900
//сигнал о готовности

pinMode(mikrik, INPUT);
//pinMode(modulestart, OUTPUT);
pinMode(startengine, OUTPUT);
pinMode(oil, INPUT);

resGlobal = 10000; //cчитать счетчик из EEPROM, присвоено тестовое значение
resOper = resGlobal;
digitalWrite(startengine, HIGH);
//писк бузера
}



void loop()
{
oilstate = digitalRead(oil);
mikrikstate = digitalRead(mikrik);

//проверяем на критические состояния

// Мониторинг питания для записи счетчика в память при выключении
if (analogRead(powerpin)< 512) 
  {
   allert = true;
   Serial.println("Power down!"); // Тестовое сообщение
   //запись значения счетчика ресурса в память 
   // писк бузера
   }

// Проверка датчика масла
else if (oilstate == HIGH)
{
  allert = true;
  Serial.println("Low oil!");
  if (smsoil = false)
  StartFirstSMS(); Serial2.println("Alarm! Utechka masla"); EndSMS(); 
 // писк бузера
//  отправка сообщения о критическом уровне масла

}
else if (resOper > 1476000)
  {
  allert = true;
//  сообщение о критической необходимости замены масла
// писк бузера 
  }
else
  {
  allert = false;
  }
if (allert == true) //блокировка двигателя при аварии
  {
  digitalWrite(startengine, HIGH);

  if((beginRes != 0) & (engine == true))
  {
    resStop = (millis() - beginRes);
    resOper = (resOper + resStop);
    Serial.println("allert engine stop, write to EEPROM = "); // тестовый вывод наработки с начала работы программы
    Serial.println(resOper);
    engine = false;
  }
  
  }
else if (mikrikstate == HIGH)//запуск двигателя при замкнутом микрике 
  {
  digitalWrite(startengine, LOW);
  stoptime = false;
  if(engine == false)
    {
      beginRes = millis(); //запуск счетчика ресурса в цикле работы двигателя
      engine = true; //переключение флага работы двигателя
      mikrikcounter = mikrikcounter ++;
    }
  } 
else if (stoptime == false)//запуск задержки остановки двигателя
  {
  stopengine = millis();

  stoptime = true;
  }
else if ((millis() - stopengine) >= 5000)//установка задержки остановки двигателя
  {
  digitalWrite(startengine, HIGH); 
  if (engine == true)
    {
    resStop = (millis() - beginRes);
    // resGlobal3 = int(resStop);
     resOper = (resOper + resStop);
     Serial.println("engine off, write to EEPROM = "); //тестовый вывод ресурса наработки со времени старта программы
     Serial.println(resOper);
     engine = false;
     }
   }
// Определение повреждения в системе высокого давления, если микрик сработал три раза за определенное время
if (mikrikstate == HIGH) // Счетчик срабатываний микрика
  {
    if (mikrikOn == false && (millis()- bounce) > 500)
     {
        bounce = millis();
        mikrikcounter = ++mikrikcounter;
        Serial.println(mikrikcounter); //тестовый вывод срабатываний микрика
        mikrikOn = true;
        if (tensionflag = false)  
         {
            tensioncounter = millis();
            tensionflag = true;
         }
     }
  }
else
  {
   mikrikOn = false;
  } 


if ((millis() - tensioncounter) >= 3000)
  {
   tensioncounter = millis();
   tensionflag = false;
   if (mikrikcounter > 2)
     { 
      mikrikcounter = 0;
      Serial.println("Tension allert"); //Тестовый отчет о проблеме в контуре высокого давления
      //бузер про обрыв контура давления
      //СМС о том же
     }   
   else 
      {
      mikrikcounter = 0;
      }
  }

/* Бузер
tone(9, 880, 500);
delay(1000);
tone(9, 1046, 500);
delay(1000);
tone(9, 1174, 500);

*/
}






/* void GSMSetup()
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

void StartFirstSMS() //__________________Цикл подготовки модуля к отправке СМС-сообщений по первому номеру
{
      Serial.print("AT+CMGF=1\r");
      delay(100);
      Serial.println("AT + CMGS = \"+380000000000\""); // (Номер сервисного центра)
      delay(100);
}      
void StartSecondSMS() //__________________Цикл подготовки модуля к отправке СМС-сообщений по второму номеру
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
