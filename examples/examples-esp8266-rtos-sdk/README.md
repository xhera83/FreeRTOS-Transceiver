### 1. Example
- Simple unidirectional comm with multiple datatypes on the queue. Also task-synchronization via the task notification feature
### 2. Template
- Follow 3) to use the template folder for your own projects

## 3. Flashing example code into the ESP8266 (guide only for linux at the moment)
1. Install docker

2. ```git clone https://github.com/xhera83/FreeRTOS-Transceiver.git```

3. ```cd /PATH_TO_FOLDER/FreeRTOS-Transceiver/examples/examples-esp8266-rtos-sdk ```

4. ```sudo docker build -f ./esp8266ImageBuild -t image8266Env```

5. ```sudo docker run -d --device=/dev/ttyUSB0 -v /PATH_TO_FOLDER/FreeRTOS-Transceiver/examples/examples-esp8266-rtos-sdk/basic-example/::/projects --privileged --name container_esp8266 image8266Env tail -f /dev/null```
    - This will, besides creating that container, mount ```.../basic-example``` on ```/projects```.
    - Of course you need to specify your own port ***/dev/tty...***
    
6. ```sudo docker exec -it container_esp8266 bash```
    - Always use this to get into the container (Check docker commands, you might need to 'start' the container again!)
    - You will now be inside the container. It will look like that: ```root@--------:/#```
    - You can still make changes to the mounted directory where the ESP8266 example is located (```../basic-example```), changes will appear inside the container!
    
7. Now we also need to have following directory/files inside ```/PATH_TO_FOLDER/FreeRTOS-Transceiver/examples/examples-esp8266-rtos-sdk/basic-example/components/``` (Changes will automatically appear inside the container!)

    - FreeRTOS-Transceiver library with:
        - ```/include``` and ```/src```
        - ```component.mk``` file
    - Two options
        - Inside the container do:  ```cd /projects/components/; git clone https://github.com/xhera83/FreeRTOS-Transceiver.git``` 
        - Outside the container in ```.../basic-example/components/``` simply create the directory ```FreeRTOS-Transceiver```and copy ```/include```, ```/src```and ```component.mk``` from the root of the repo into it
    - Must look like this inside the ```/projects/compontents``` OR ```.../basic-example``` folder:
        
               
                    FreeRTOS-Transceiver/
                            - /include
                            - /src
                            - component.mk
                    component1/
                            - ....
                    component2/
                            - ....
                            
 8. Now, if you're still inside the container (else revisit 6)) type ``` make help ``` for all commands:
    - ```make menuconfig``` -  A GUI will open where you can set things up (I didnt need to)
    - ``` make clean``` - Clean old build
    - ``` make app ``` - Compiles app etc.
    - ``` make flash monitor ``` - Flash + console output right away.
    
    
    
    
 ## 3. Additional informations
    
- Projects are  set up in ESP-IDF style: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html
- The project folder structure of an ESP8266 project : https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/api-guides/build-system.html
- FRTTransceiver library uses C++, so all Files should be with an .cpp extension. 
- Main.cpp's app_main() (like loop(), entry point for your projects software) function must be treated as a C function. Enclose as follows:
            
        ......
        using namespace FRTT;
        
        
        #ifdef __cplusplus
        extern "C"{
        #endif

        void app_main()
        {   

            // Very first code of your project. Here you can use the library as you would do for the ESP32 (but only 1 Core)

            QUEUE_TO_TASK2 = FRTTCreateQueue(2);                                                // Create queue with length 2
            SMPH = FRTTCreateSemaphore();                                                       // Create semaphore
            FRTTCreateTask(TASK1,"task-1",STACKSIZE,NULL,8,&TASK1_HANDLE);                      // Creates TASK1 (esp8266 only function)
            FRTTCreateTask(TASK2,"task-2",STACKSIZE,NULL,8,&TASK2_HANDLE);                      // Creates TASK2 (esp8266 only function)
        }

        #ifdef __cplusplus
        }
        #endif


                            
              


