/*
  Tim Hsu, <timhsu@chroot.org>, June 2013.

  insmod the module to disable SMEP, or enable SMEP again when rmmod this.
  
*/

#include <linux/module.h>
#include <linux/kernel.h>

#define X86_CR4_SMEP	0x00100000 /* enable SMEP support */

MODULE_AUTHOR("Tim Hsu, <timhsu@chroot.org>");
MODULE_DESCRIPTION("Disable/Enable SMEP of CPU feature.");
MODULE_LICENSE("Apache License v2.0");

static int detect_cpu_smep(void)
{
	unsigned int eax, ebx, ecx, edx;

	eax = 7;
	ecx = 0;
	asm volatile("cpuid"
		: "=a" (eax),
		  "=b" (ebx),
		  "=c" (ecx),
		  "=d" (edx)
		: "0" (eax), "2" (ecx));

	if (ebx & (1 << 7)) {
		return 1;
	}
	return 0;
}

static unsigned long _read_cr4(void)
{
  unsigned long _cr4;
  asm volatile ("mov %%cr4, %0"
                : "=r" (_cr4)
  );
  return _cr4;
}

static void _write_cr4(unsigned long _cr4)
{

  asm volatile ("mov %0,%%cr4"
                : : "r" (_cr4)
  );

}

static int to_disable_smep(void)
{
  unsigned long _cr4;
  
  _cr4 = _read_cr4();
//  printk(KERN_INFO "CR4 = 0x%lx", _cr4);
  if (_cr4 & X86_CR4_SMEP) {
  
    _cr4 = (_cr4 ^ X86_CR4_SMEP);
    _write_cr4(_cr4);   
    return 1;
  }         
  return 0;                      
}

static int to_enable_smep(void)
{
  unsigned long _cr4;
  
  _cr4 = _read_cr4();
//  printk(KERN_INFO "CR4 = 0x%lx", _cr4);

  if (detect_cpu_smep() & !(_cr4 & X86_CR4_SMEP)) {
    _cr4 = (_cr4 | X86_CR4_SMEP);   
    _write_cr4(_cr4);
    return 1;
  }   
  return 0;
}


extern 
int init_module(void)
{

  if (to_disable_smep()) {
    printk(KERN_INFO "Disabled SMEP!\n");
  } else {
    printk(KERN_INFO "No SMEP feature.\n");
  }
  return 0;
}

void cleanup_module(void)
{
  if (to_enable_smep()) {
    printk(KERN_INFO "Enabled SMEP!\n");
  } else {
    printk(KERN_INFO "No SMEP feature.\n");  
  }
}
