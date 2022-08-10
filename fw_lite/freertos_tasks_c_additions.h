
#include "platform.h"

void print_stack(uint32_t *stk, size_t n)
{
	for (uint32_t i = 0; i < n; i++) {
		uart_printf("%02d: %08x\n", -i, stk[n-i]);
	}
}

void tasks_print(void)
{
	TaskStatus_t *pxTaskStatusArray;
	UBaseType_t uxArraySize, x;
	unsigned long ulTotalRunTime, ulStatsAsPercentage;

	uxArraySize = uxTaskGetNumberOfTasks();

	pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );
	configASSERT(pxTaskStatusArray != NULL );

	uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );
	for( x = 0; x < uxArraySize; x++ ) {
		TaskStatus_t *t = &pxTaskStatusArray[x];
		tskTCB *tcb = t->xHandle;

		uart_printf("%s\t%p\n", t->pcTaskName, t->xHandle);

//		print_stack(tcb->pxTopOfStack, 20);

	}

	uart_printf("current %p\n", pxCurrentTCB);

const uint32_t ulICCIAR = portICCIAR_INTERRUPT_ACKNOWLEDGE_REGISTER_ADDRESS;
const uint32_t ulICCEOIR = portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS;
const uint32_t ulICCPMR	= portICCPMR_PRIORITY_MASK_REGISTER_ADDRESS;
uart_printf("iar = %p eoi= %p pmr = %p\n", ulICCIAR, ulICCEOIR, ulICCPMR);
uart_printf("iar = %p eoi= %p pmr = %p\n", &GICInterface->IAR, &GICInterface->EOIR, &GICInterface->PMR);

}
