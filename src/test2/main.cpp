#include <Arduino.h>

TaskHandle_t Task1, Task2;
uint64_t ISRcnt[2] = {0, 0};
uint32_t ISRmrk[2] = {0, 0};

#define BTN_1 GPIO_NUM_25
#define BTN_2 GPIO_NUM_26

#define TST_1 GPIO_NUM_32
#define TST_2 GPIO_NUM_33

class InterruptButton
{
  // private:
  // gpio_num_t gpio;
  // gpio_mode_t gpioDirection;
  // gpio_pull_mode_t pullMode;
  // gpio_int_type_t interruptType;

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


void IRAM_ATTR count1_isr(void *args)
{
  ISRcnt[0] += 1;
//   ISRmrk[0] = micros();
}

void IRAM_ATTR count2_isr(void *args)
{
  ISRcnt[1] += 1;
  ISRmrk[1] = millis();
}

void _task1func(void *params)
{
  for (;;)
  {
    esp_err_t allocationStatus = esp_intr_alloc(ETS_GPIO_INTR_SOURCE, 0, count1_isr, NULL, NULL);
    // uxTaskGetStackHighWaterMark
    int cpu_id = esp_intr_get_cpu((intr_handle_t)count1_isr);

    if(ISRcnt[0] > 50)
    {
      ISRcnt[0] = 0;
      ISRmrk[0] = micros() - ISRmrk[0];
    
    }
    else
    {
      Serial.println(ISRmrk[1] + "\t" + (micros() - ISRmrk[0]));
    }
  
    Serial.print("Allocation Result: ");
    Serial.println(esp_err_to_name(allocationStatus));
    Serial.print("count1 ISR running on Core id: ");
    Serial.println(cpu_id);
  }

  //gpio_uninstall_isr_service
  //vTaskDelete(NULL);
}

void _task2func(void *params)
{
  for (;;)
  {
    esp_err_t allocationStatus = esp_intr_alloc(ETS_GPIO_INTR_SOURCE, 0, count2_isr, NULL, NULL);
    int cpu_id = esp_intr_get_cpu((intr_handle_t)count2_isr);

    Serial.print("Allocation Result: ");
    Serial.println(esp_err_to_name(allocationStatus));
    Serial.print("count2 ISR running on Core id: ");
    Serial.println(cpu_id);
  }
}

void tst1_init()
{
  gpio_set_direction(TST_1, GPIO_MODE_OUTPUT);
}

void tst2_init()
{
  gpio_set_direction(TST_2, GPIO_MODE_OUTPUT);
}

void btn1_init(void)
{

  button1.init(BTN_1, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY, GPIO_INTR_NEGEDGE);
  gpio_isr_handler_add(BTN_1, count1_isr, NULL);
}

void btn2_init(void)
{

  button2.init(BTN_2, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY, GPIO_INTR_NEGEDGE);
  gpio_isr_handler_add(BTN_2, count1_isr, NULL);
}



void setup()
{
  //Install the driver’s GPIO ISR handler service, which allows per-pin GPIO interrupt handlers.
  gpio_install_isr_service(0);
  btn1_init();
  btn2_init();
  tst1_init();
  tst2_init();

  Serial.begin(115200);

  xTaskCreatePinnedToCore(
      _task1func,
      "increment count1",
      8192,
      NULL,
      1,
      &Task1,
      0);

  delay(500); // needed to start-up task1

  xTaskCreatePinnedToCore(
      _task2func,
      "increment count2",
      8192, //stack depth
      NULL, //passing params to task
      2,    //Priority
      &Task2,
      1 //Core ID
  );
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(Task1);
  Serial.print("Task 1 Stack Size: ");
  Serial.println(uxHighWaterMark);
  uxHighWaterMark = uxTaskGetStackHighWaterMark(Task2);
  Serial.print("Task 2 Stack Size: ");
  Serial.println(uxHighWaterMark);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(500);
}