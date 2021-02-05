//Test Sketch by @Sandeepan
#include <Arduino.h>

TaskHandle_t Task1, Task2;
int count1 = 0, lastCount1 = 0, count2 = 0, lastCount2 = 0;

#define BTN_1 GPIO_NUM_18
#define BTN_2 GPIO_NUM_19

#define LED_1 GPIO_NUM_25
#define LED_2 GPIO_NUM_26

class InterruptButton
{
public:
  void init(gpio_num_t _gpio,
            gpio_mode_t _gpioDirection,
            gpio_pull_mode_t _pullMode,
            gpio_int_type_t _interruptType)
  {
    gpio_set_direction(_gpio, _gpioDirection);
    gpio_set_pull_mode(_gpio, _pullMode);
    gpio_set_intr_type(_gpio, _interruptType);
  }
};

InterruptButton button1, button2;

typedef enum
{
  OFF,
  ON
} pin_logic;
void digWrite(gpio_num_t gpio, pin_logic logic)

{

  gpio_set_level(gpio, (uint32_t)logic);
}

void IRAM_ATTR count1_isr(void *args)
{
  count1++;
}

void IRAM_ATTR count2_isr(void *args)
{
  count2++;
}

void _task1func(void *params)
{
  String taskMessage = "\nTask 1 running on core ";
  taskMessage = taskMessage + xPortGetCoreID();

  while (true)
  {
    if (count1 == 10)
      count1 = 0;
    if (count1 != lastCount1)
    {
      taskMessage += "\tCount1: " + String(count1);
      taskMessage += "\tTimeStamp: " + String(millis());
      Serial.println(taskMessage);
      taskMessage="";
      UBaseType_t uxHighWaterMark;
      uxHighWaterMark = uxTaskGetStackHighWaterMark(Task1);
      Serial.print("Task 1 Stack Size: ");
      Serial.println(uxHighWaterMark);
      lastCount1 = count1;
    }
    // delay(10);
  }

}

void _task2func(void *params)
{
  String taskMessage = "\nTask 2 running on core ";
  taskMessage = taskMessage + xPortGetCoreID();

  for (;;)
  {

    if (count2 == 10)
      count2 = 0;
    if (count2 != lastCount2)
    {
      taskMessage += "\tCount2: " + String(count2);
      taskMessage += "\tTimeStamp: " + String(millis());
      Serial.println(taskMessage);
      taskMessage = "";
      UBaseType_t uxHighWaterMark;
      uxHighWaterMark = uxTaskGetStackHighWaterMark(Task2);
      Serial.print("Task 2 Stack Size: ");
      Serial.println(uxHighWaterMark);
      lastCount2 = count2;
    }
    // delay(10);
  }
}

void btn1_init(void)
{

  button1.init(BTN_1, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY, GPIO_INTR_NEGEDGE);
  gpio_isr_handler_add(BTN_1, count1_isr, NULL);
}

void btn2_init(void)
{

  button2.init(BTN_2, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY, GPIO_INTR_NEGEDGE);
  gpio_isr_handler_add(BTN_2, count2_isr, NULL);
}

void setup()
{
  Serial.begin(115200);
  //Install the driver’s GPIO ISR handler service, which allows per-pin GPIO interrupt handlers.
  gpio_install_isr_service(0);
  btn1_init();
  btn2_init();

  xTaskCreatePinnedToCore(
      _task1func,
      "increment count1",
      12000,
      NULL,
      1,
      &Task1,
      0);
  delay(500); // needed to start-up task1

  xTaskCreatePinnedToCore(
      _task2func,
      "increment count2",
      12000, //stack depth
      NULL,  //passing params to task
      2,     //Priority
      &Task2,
      1 //Core ID
  );  
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Starting main loop...");
  while (true)
  {
    ; //Placeholder
  }
}