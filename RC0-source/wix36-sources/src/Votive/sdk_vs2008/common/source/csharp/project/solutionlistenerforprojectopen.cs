/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Security;
using System.Windows;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using System.Collections;
using System.Text;
using IOleServiceProvider = Microsoft.VisualStudio.OLE.Interop.IServiceProvider;
using IServiceProvider = System.IServiceProvider;
using ShellConstants = Microsoft.VisualStudio.Shell.Interop.Constants;
using OleConstants = Microsoft.VisualStudio.OLE.Interop.Constants;

namespace Microsoft.VisualStudio.Package
{

	[CLSCompliant(false)]
	public class SolutionListenerForProjectOpen : SolutionListener
	{
		public SolutionListenerForProjectOpen(IServiceProvider serviceProvider) : base(serviceProvider)
		{
		}
		
		public override int OnAfterOpenProject(IVsHierarchy hierarchy, int added)
		{
			// If this is a new project and our project. We use here that it is only our project that will implemnet the "internal"  IBuildDependencyOnProjectContainer.
			if (added != 0 && hierarchy is IBuildDependencyUpdate)
			{
				IVsUIHierarchy uiHierarchy = hierarchy as IVsUIHierarchy;
				Debug.Assert(uiHierarchy != null, "The ProjectNode should implement IVsUIHierarchy");
				// Expand and select project node
				IVsUIHierarchyWindow uiWindow = UIHierarchyUtilities.GetUIHierarchyWindow(this.ServiceProvider, HierarchyNode.SolutionExplorer);
				if (uiWindow != null)
				{
				    __VSHIERARCHYITEMSTATE state;
				    uint stateAsInt;
					if (uiWindow.GetItemState(uiHierarchy, VSConstants.VSITEMID_ROOT, (uint)__VSHIERARCHYITEMSTATE.HIS_Expanded, out stateAsInt) == VSConstants.S_OK)
				    {
				        state = (__VSHIERARCHYITEMSTATE)stateAsInt;
				        if (state != __VSHIERARCHYITEMSTATE.HIS_Expanded)
				        {
				            int hr;
							hr = uiWindow.ExpandItem(uiHierarchy, VSConstants.VSITEMID_ROOT, EXPANDFLAGS.EXPF_ExpandParentsToShowItem);
				            if (ErrorHandler.Failed(hr))
				                Trace.WriteLine("Failed to expand project node");
							hr = uiWindow.ExpandItem(uiHierarchy, VSConstants.VSITEMID_ROOT, EXPANDFLAGS.EXPF_SelectItem);
				            if (ErrorHandler.Failed(hr))
				                Trace.WriteLine("Failed to select project node");

				            return hr;
				        }
				    }
				}
			}
			return VSConstants.S_OK;
		}

        /// <summary> 
        /// Called at load time when solution has finished opening. 
        /// The ProjectLoadDialogState is set to Show again for unsafe projects 
        /// that are added to the solution after an eventually safe project has cleaned it.
        /// </summary>
        /// <param name="reserved">reserved</param>
        /// <param name="isSolution">true if this is a new solution</param>
        /// <returns>Success or a filure code.</returns>
        public override int OnAfterOpenSolution(object reserved, int isSolution)
        {
            // Once the solution is open, throw away any saved dialog responses so that if the
            // user now does an Add Existing Project (on an insecure project), he should see the
            // security warning dialog again.  Unchecking the "Ask me always" checkbox only
            // applies to the loading of the solution.  Subsequent project loads show the dialog again.
            Utilities.SaveDialogStateInSolution(this.ServiceProvider, _ProjectLoadSecurityDialogState.PLSDS_ShowAgain);

            return VSConstants.S_OK;
        }

	}
}
