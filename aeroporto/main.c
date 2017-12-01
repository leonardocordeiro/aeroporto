#include <stdlib.h>
#include <stdio.h>
#include <Strings.h>
#include <curses.h>

typedef struct _airplane {
    int fuel;
    int id;
    int timeToWaitForLanding;
    int timeToWaitForTakeoff;
    
} Airplane;

typedef struct _node {
    Airplane* airplane;
    
    struct _node* next;
    struct _node* previous;
    
} Node;

typedef struct _queue {
    int size;
    
    struct _node* last;
    struct _node* first;
} Queue;

typedef struct _strip {
    struct _airplane* airplane;
    int landing;
    
} Strip;

int randomNumber(int start, int end);
void calculateTimeToWaitForLanding(Queue** queues, int queueQuantity, Airplane* airplane);
void calculateTimeToWaitForTakeOff(Queue** queues, int queuesQuantity, Airplane* airplane);
int checkBetterQueue(Queue** queue, int queueQuantity);

int fuelQuantity(Airplane* airplane) {
    return airplane->fuel;
}

int isEmpty(Queue* queue) {
    if(queue->size == 0 && queue->last == NULL) return 1;
    return 0;
}

Queue* newQueue() {
    Queue* queue = malloc(sizeof(Queue));
    
    queue->first = NULL;
    queue->last = NULL;
    queue->size = 0;
    
    return queue;
}

Strip* newLandingStrip() {
    Strip* landingStrip = malloc(sizeof(Strip));
    landingStrip->airplane = NULL;
    landingStrip->landing = 1;
    
    return landingStrip;
}

Airplane* newAirplane(int fuel) {
    Airplane* airplane = malloc(sizeof(Airplane));
    airplane->fuel = fuel;
    airplane->id = randomNumber(1, 60000);
    airplane->timeToWaitForLanding = -1;
    airplane->timeToWaitForTakeoff = -1;
    
    return airplane;
}

Airplane* pop(Queue* queue) {
    if(!isEmpty(queue)) {
        Node* first = queue->first;
        queue->first = first->previous;
        
        if(queue->first == NULL)
            queue->last = NULL;
        
        queue->size--;
        return first->airplane;
    } else {
        return NULL;
    }
    
}

void freeQueue(Queue* queue) {
    for(Node* current = queue->first;
        current != NULL;
        current = current->previous) {
        
        free(current);
        
    }
    
    free(queue);
    queue = NULL;
}

void add(Airplane* airplane, Queue* queue) {
    if(airplane != NULL && queue != NULL) {
        Node* node = (Node*) malloc(sizeof(Node));
        node->next = NULL;
        node->previous = NULL;
        node->airplane = airplane;
        
        if(isEmpty(queue)) {
            node->next = NULL;
            node->previous = NULL;
            
            queue->first = node;
            queue->last = node;
        } else {
            Node* currentLast = queue->last;
            
            currentLast->previous = node;
            node->next = currentLast;
            
        }
        queue->last = node;
        queue->size++;
    }
}



void showAll(Queue** queues, int queueQuantity, int landing) {
    for(int i = 0; i < queueQuantity; i++) {
        printf("Fila %i (%i): ", i, queues[i]->size);
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            if(landing)
                calculateTimeToWaitForLanding(queues, queueQuantity, current->airplane);
            else
                calculateTimeToWaitForTakeOff(queues, queueQuantity, current->airplane);
            
            if(current->airplane->timeToWaitForLanding > -1) {
                printf("[id: %i, combustivel: %i, espera-pouso: %i] ", current->airplane->id, current->airplane->fuel, current->airplane->timeToWaitForLanding);
                
            } else if(current->airplane->timeToWaitForLanding == -1) {
                printf("[id: %i, combustivel: %i, espera-decolagem: %i] ", current->airplane->id, current->airplane->fuel, current->airplane->timeToWaitForTakeoff);
            }
            
        }
        printf("\n");
    }
}

int randomNumber(int start, int end) {
    return start + ( arc4random() % ++end);
}

Queue** createQueues(int quantity) {
    Queue** queues = malloc(quantity * sizeof(Queue*));
    
    for(int i = 0; i < quantity; i++) {
        queues[i] = newQueue();
    }
    
    return queues;
}

Strip** createLandingStrips(int quantity) {
    Strip** landingStrips = malloc(quantity * sizeof(Strip*));
    
    for(int i = 0; i < quantity; i++) {
        landingStrips[i] = newLandingStrip();
        if(i == (quantity - 1)) landingStrips[i]->landing = 0;
    }
    
    return landingStrips;
}

void createAirplanesOn(Queue** queue, int quantity, int landing, int queueQuantity) {
    
    int queueIndex = checkBetterQueue(queue, queueQuantity);
    printf("=====================================================");
    printf("\n%i novo(s) aviões na fila %i\n\n", quantity, queueIndex);
    
    for(int i = 0; i < quantity; i++) {
        int fuel = landing ? randomNumber(1, 19) : 20;
        Airplane* airplane = newAirplane(fuel);
        
        if(landing && (airplane->id % 2 != 0))
            airplane->id++;
        
        if(!landing && (airplane->id % 2 == 0))
            airplane->id++;
        
        printf("[id: %i, combustível: %i] \n\n", airplane->id, airplane->fuel);
        add(airplane, queue[queueIndex]);
        
    }
}

void decrementFuel(Airplane* airplane) {
    if(airplane != NULL)
        airplane->fuel--;
}

int checkBetterQueue(Queue** queue, int queueQuantity) {
    int currentSize = 0;
    int betterQueue = 0;
    
    for(int i = 0; i < queueQuantity; i++) {
        int queueSize = queue[i]->size;
       
        if(queueSize < currentSize)
            betterQueue = i;

        currentSize = queueSize;
    }
    
    return betterQueue;
}

void waitCommand() {
    getchar();
}

void removeLast(Queue* queue) {
    if(queue != NULL) {
        if(queue->size == 1) pop(queue);
        else {
            queue->last->next->previous = NULL;
            queue->last = queue->last->next;
            queue->size--;
        
        }
    }
}

void removeFrom(Queue* queue, Airplane* airplane) {
    int i = 0;
    if(queue != NULL) {
        for(Node* current = queue->first;
            current != NULL;
            current = current->previous, i++) {
            
            if(current->airplane->id == airplane->id) {
                
                if(queue->size == 1 || i == 0) pop(queue);
                else if(i == queue->size - 1) {
                    removeLast(queue);
                } else {
                    current->previous->next = current->next;
                    current->next->previous = current->previous;
                    queue->size--;
                }
                
            }
            
        }
    }
}

Airplane* getLessFuelAirplane(Queue** queues, int queuesQuantity) {
    int smallestFuel = 22;
    Airplane* lessFuelAirplane = NULL;
    Queue* queueOfLessFuelAirplane = NULL;
    
    for(int i = 0; i < queuesQuantity; i++) {
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            Airplane* currentAirplane = current->airplane;
            if(currentAirplane->fuel < smallestFuel) {
                
                lessFuelAirplane = currentAirplane;
                queueOfLessFuelAirplane = queues[i];
                smallestFuel = currentAirplane->fuel;
            }
            
        }
    }
    
    return lessFuelAirplane;
    
}

Airplane* removeLessFuelAirplane(Queue** queues, int queuesQuantity) {
    int smallestFuel = 22;
    Airplane* lessFuelAirplane = NULL;
    Queue* queueOfLessFuelAirplane = NULL;
    
    for(int i = 0; i < queuesQuantity; i++) {
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            Airplane* currentAirplane = current->airplane;
            if(currentAirplane->fuel < smallestFuel) {
                
                lessFuelAirplane = currentAirplane;
                queueOfLessFuelAirplane = queues[i];
                smallestFuel = currentAirplane->fuel;
            }
           
        }
    }
   
    removeFrom(queueOfLessFuelAirplane, lessFuelAirplane);
    return lessFuelAirplane;
    
}

void eraseLandingStrips(Strip** landingStrips, int landingStripsQuantity) {
    
    for(int i = 0; i < landingStripsQuantity; i++) {
        landingStrips[i]->airplane = NULL;
    }
    
}

void toLandLessFuelAirplaneOn(Queue** queues, int queuesQuantity, Strip** landingStrips, int landingStripsQuantity) {
    
    Airplane* lessFuelAirplane = removeLessFuelAirplane(queues, queuesQuantity);
    int i = 0;
    for(i = 0; i < landingStripsQuantity; i++) {
        Airplane* currentAirplane = landingStrips[i]->airplane;
        if(currentAirplane == NULL) {
            if(landingStrips[i]->landing == 1)
                landingStrips[i]->airplane = lessFuelAirplane;
            return;
        }
    }
    
    if(i == landingStripsQuantity) {
        eraseLandingStrips(landingStrips, landingStripsQuantity);
    }
    
}

// strip quantity == queues quantity!!!
void takeOffAirplanes(Queue** queues, int queuesQuantity, Strip** landingStrips) {

    Airplane** airplanesToTakeOff = malloc(queuesQuantity * sizeof(Airplane*));
    
    for(int i = 0; i < queuesQuantity; i++) {
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            Airplane* lessFuelAirplane = getLessFuelAirplane(queues, queuesQuantity);
            if(lessFuelAirplane->fuel <= 1) {
                return;
            }
        }
        
        Airplane* airplaneToTakeOff = pop(queues[i]);
        airplanesToTakeOff[i] = airplaneToTakeOff;
    }
    
    for(int i = 0; i < queuesQuantity; i++) {
        landingStrips[i]->airplane = airplanesToTakeOff[i];
    }
    
}

void calculateTimeToWaitForTakeOff(Queue** queues, int queuesQuantity, Airplane* airplane) {
    
    int queuePosition = 0;

    for(int i = 0; i < queuesQuantity; i++) {
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            current->airplane->timeToWaitForTakeoff = queuePosition++;
        }
        queuePosition = 0;
    }
    
}

void calculateTimeToWaitForLanding(Queue** queues, int queuesQuantity, Airplane* airplane) {
    
    int airplanesQuantity = 0;
    for(int i = 0; i < queuesQuantity; i++) {
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            if(current->airplane != NULL)
                airplanesQuantity++;
            
        }
    }
    
    Airplane** airplanes = malloc(airplanesQuantity * sizeof(Airplane*));
    int k = 0;
    for(int i = 0; i < queuesQuantity; i++) {
        for(Node* current = queues[i]->first;
            current != NULL;
            current = current->previous) {
            
            Airplane* airplane = current->airplane;
            airplanes[k++] = airplane;
            
        }
    }
    
    int aux = 0;
    for(int i = 0; i < airplanesQuantity - 1; i++) {
        for(int j = i + 1; j < airplanesQuantity; j++) {
            if(airplanes[i] != NULL && airplanes[j] != NULL) {
                if(airplanes[i]->fuel > airplanes[j]->fuel) {
                    aux = airplanes[i]->fuel;
                    airplanes[i]->fuel = airplanes[j]->fuel;
                    airplanes[j]->fuel = aux;
                }
            }
        }
    }
    
    for(int i = 0; i < airplanesQuantity; i++) {
        if(airplanes[i]->id == airplane->id) {
            airplanes[i]->timeToWaitForLanding = i;
        }
    }
    
    
}

void showAllLandingStrip(Strip** landingStrip, int landingStripQuantity) {
    
    for(int i = 0; i < landingStripQuantity; i++) {
        printf("\nPista %i: ", (i + 1));
        if(landingStrip[i]->airplane != NULL) {
            
            if(landingStrip[i]->airplane->fuel < 20) {
                printf("[id: %i, combustivel: %i, status: Pouso] \n", landingStrip[i]->airplane->id, landingStrip[i]->airplane->fuel);
            } else {
                printf("[id: %i, combustivel: %i, status: Decolagem] \n", landingStrip[i]->airplane->id, landingStrip[i]->airplane->fuel);
            }
        
        }
    }
}

int main() {
    Queue** landingQueue = createQueues(4);
    Queue** takeOffQueue = createQueues(3);
    
    Strip** landingStrips = createLandingStrips(3);
    
    for(int time = 0;; time++) {
        waitCommand();

        int airplaneLandingNumber = randomNumber(0, 3);
        int airplaneTakeOffNumber = randomNumber(0, 3);

        createAirplanesOn(landingQueue, airplaneLandingNumber, 1, 4);
        createAirplanesOn(takeOffQueue, airplaneTakeOffNumber, 0, 3);
        
        takeOffAirplanes(takeOffQueue, 3, landingStrips);
        toLandLessFuelAirplaneOn(landingQueue, 4, landingStrips, 3);
        
        printf("\nFilas de pouso->\n");
        showAll(landingQueue, 4, 1);

        printf("\nFilas de decolagem->\n");
        showAll(takeOffQueue, 3, 0);
        showAllLandingStrip(landingStrips, 3);

    }
    return 0;
}
