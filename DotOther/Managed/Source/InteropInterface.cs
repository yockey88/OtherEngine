using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Reflection;

namespace DotOther.Managed.Interop {

using static DotOtherHost;

	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	public readonly struct InternalCall {
		public readonly IntPtr NName;
		public readonly IntPtr Target;

		public string? Name => Marshal.PtrToStringAnsi(NName);
	}

}