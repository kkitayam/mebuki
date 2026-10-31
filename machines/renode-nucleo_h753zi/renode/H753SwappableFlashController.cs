// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 Koji KITAYAMA

using System.Linq;

using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Peripherals.Memory;

namespace Antmicro.Renode.Peripherals.MTD
{
    public class H753SwappableFlashController : STM32H7_FlashController
    {
        public H753SwappableFlashController(IMachine machine, MappedMemory flash1, MappedMemory flash2)
            : base(machine, flash1, flash2)
        {
            this.machine = machine;
            this.flash1 = flash1;
            this.flash2 = flash2;
            committedOptions = base.ReadDoubleWord(OptionStatusCurrent);
        }

        public override void Reset()
        {
            if(machine != null)
            {
                var programmed = base.ReadDoubleWord(OptionStatusProgram);
                if(programmed != committedOptions)
                {
                    if((base.ReadDoubleWord(OptionControl) & 1) != 0)
                    {
                        base.WriteDoubleWord(OptionKey, 0x08192A3B);
                        base.WriteDoubleWord(OptionKey, 0x4C5D6E7F);
                    }
                    base.WriteDoubleWord(OptionStatusProgram, committedOptions);
                }
            }
            base.Reset();
            if(machine != null)
            {
                SetMapping((committedOptions & SwapBit) != 0);
            }
        }

        public override uint ReadDoubleWord(long offset)
        {
            return base.ReadDoubleWord(TranslateBankRegister(offset));
        }

        public override void WriteDoubleWord(long offset, uint value)
        {
            var translatedOffset = TranslateBankRegister(offset);
            if((translatedOffset == 0x0C || translatedOffset == 0x10C) &&
                (base.ReadDoubleWord(translatedOffset) & 1) != 0)
            {
                this.Log(LogLevel.Warning, "Ignoring a write to a locked flash bank control register.");
                return;
            }
            var programOptions = offset == OptionControl && (value & 2) != 0 &&
                (base.ReadDoubleWord(OptionControl) & 1) == 0;
            base.WriteDoubleWord(translatedOffset, value);
            if(programOptions)
            {
                committedOptions = base.ReadDoubleWord(OptionStatusCurrent);
            }
        }

        private long TranslateBankRegister(long offset)
        {
            var bankOffset = offset & ~0x100L;
            if(swapped && (bankOffset == 0x04 || bankOffset == 0x0C ||
                bankOffset == 0x10 || bankOffset == 0x14 ||
                (bankOffset >= 0x28 && bankOffset <= 0x3C) ||
                (bankOffset >= 0x50 && bankOffset <= 0x58) || bankOffset == 0x60))
            {
                return offset ^ 0x100;
            }
            return offset;
        }

        private void SetMapping(bool desired)
        {
            if(desired == swapped)
            {
                return;
            }

            var bus = (SystemBus)machine.SystemBus;
            var firstAddress = swapped ? Bank2Address : Bank1Address;
            var secondAddress = swapped ? Bank1Address : Bank2Address;
            var registrations = bus.GetRegistrationsForPeripheralType<MappedMemory>().ToArray();
            var first = registrations.Single(x => x.Peripheral == flash1 && x.RegistrationPoint.Range.StartAddress == firstAddress);
            var second = registrations.Single(x => x.Peripheral == flash2 && x.RegistrationPoint.Range.StartAddress == secondAddress);
            var bootAlias = registrations.Single(x =>
                x.Peripheral == (swapped ? flash2 : flash1) &&
                x.RegistrationPoint.Range.StartAddress == 0);
            if(!hasAliases)
            {
                bus.Register(flash1, new BusRangeRegistration(Alias1Address, BankSize));
                bus.Register(flash2, new BusRangeRegistration(Alias2Address, BankSize));
                hasAliases = true;
            }
            bus.Unregister(first);
            bus.Unregister(second);
            bus.Unregister(bootAlias);
            bus.Register(flash1, new BusRangeRegistration(secondAddress, BankSize));
            bus.Register(flash2, new BusRangeRegistration(firstAddress, BankSize));
            bus.Register(desired ? flash2 : flash1, new BusRangeRegistration(0, BankSize));
            swapped = desired;
        }

        private const long OptionKey = 0x08;
        private const long OptionControl = 0x18;
        private const long OptionStatusCurrent = 0x1C;
        private const long OptionStatusProgram = 0x20;
        private const uint SwapBit = 1u << 31;
        private const ulong Bank1Address = 0x08000000;
        private const ulong Bank2Address = 0x08100000;
        private const ulong Alias1Address = 0x08200000;
        private const ulong Alias2Address = 0x08300000;
        private const ulong BankSize = 0x100000;
        private readonly IMachine machine;
        private readonly MappedMemory flash1;
        private readonly MappedMemory flash2;
        private bool hasAliases;
        private bool swapped;
        private uint committedOptions;
    }
}
