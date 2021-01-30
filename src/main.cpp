#include <Arduino.h>

TaskHandle_t Task1, Task2;
int count1 = 0, count2 = 0;

#define BTN_1 GPIO_NUM_10
#define BTN_2 GPIO_NUM_11

#define LED_1 GPIO_NUM_12
#define LED_2 GPIO_NUM_13

void IRAM_ATTR count1_isr(void *args)
{
  count1++;
}

void IRAM_ATTR count2_isr(void *args)
{
  count2++;
}

void blink_times(gpio_num_t led, byte times)
{
  if (times > 0)
  {
    for (int i = 0; i < times; i++)
    {
      gpio_set_level(led, 1);
      delay(500);
      gpio_set_level(led, 0);
      delay(500);
    }
  }
}

void _task1func(void *params)
{
  for (;;)
  {
    esp_err_t allocationStatus = esp_intr_alloc(ETS_GPIO_INTR_SOURCE, 0, count1_isr, NULL, NULL);
    // uxTaskGetStackHighWaterMark
    int cpu_id = esp_intr_get_cpu((intr_handle_t)count1_isr);

    Serial.print("Allocation Result: ");
    Serial.println(esp_err_to_name(allocationStatus));
    Serial.print("count1 ISR running on Core id: ");
    Serial.println(cpu_id);

    if (count1 == 10)
      count1 = 0;
    blink_times(LED_1, (byte)count1);
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

    if (count2 == 10)
      count2 = 0;
    blink_times(LED_2, (byte)count2);
  }
}

void led1_init()
{
  gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
}

void led2_init()
{
  gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
}

void btn1_init(void)
{
  gpio_pullup_en(BTN_1);
  gpio_set_direction(BTN_1, GPIO_MODE_INPUT);
  gpio_set_intr_type(BTN_1, GPIO_INTR_NEGEDGE);
  //gpio_install_isr_service(0);
  gpio_isr_handler_add(BTN_1, count1_isr, NULL);
}

void btn2_init(void)
{
  gpio_pullup_en(BTN_2);
  gpio_set_direction(BTN_2, GPIO_MODE_INPUT);
  gpio_set_intr_type(BTN_2, GPIO_INTR_NEGEDGE);
  //gpio_install_isr_service(0);
  gpio_isr_handler_add(BTN_2, count1_isr, NULL);
}

void setup()
{

  // gpio_config_t buttonConfig;

  // buttonConfig.intr_type = GPIO_INTR_NEGEDGE;
  // buttonConfig.mode = GPIO_MODE_INPUT;
  // buttonConfig.pull_down_en = 0;
  // buttonConfig.pull_up_en = 1;
  //Install the driver’s GPIO ISR handler service, which allows per-pin GPIO interrupt handlers.
  gpio_install_isr_service(0);
  btn1_init();
  btn2_init();
  led1_init();
  led2_init();

  Serial.begin(115200);

  xTaskCreatePinnedToCore(
      _task1func,
      "increment count1",
      1024,
      NULL,
      1,
      &Task1,
      0);
  delay(500); // needed to start-up task1
  xTaskCreatePinnedToCore(
      _task2func,
      "increment count2",
      1024, //stack depth
      NULL, //passing params to task
      1,    //Priority
      &Task2,
      1 //Core ID
  );
    UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(Task1);
  Serial.print("Task 1 Stack Size: ");Serial.println(uxHighWaterMark);
  uxHighWaterMark = uxTaskGetStackHighWaterMark(Task2);
  Serial.print("Task 2 Stack Size: ");Serial.println(uxHighWaterMark);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(500);
}