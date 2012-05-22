/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Designer.Interfaces;
using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;

using IOleServiceProvider = Microsoft.VisualStudio.OLE.Interop.IServiceProvider;
using IServiceProvider = System.IServiceProvider;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.VisualStudio.Package
{

	/// <summary>
	/// All public properties on Nodeproperties or derived classes are assumed to be used by Automation by default.
	/// Set this attribute to false on Properties that should not be visible for Automation.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property, AllowMultiple = false, Inherited = true)]
	public class AutomationBrowsableAttribute : System.Attribute
	{
		public AutomationBrowsableAttribute(bool browsable)
		{
			this.browsable = browsable;
		}

		public bool Browsable
		{
			get
			{
				return this.browsable;
			}
		}

		private bool browsable;
	}

	/// <summary>
	/// To create your own localizable node properties, subclass this and add public properties
	/// decorated with your own localized display name, category and description attributes.
	/// </summary>
	[CLSCompliant(false), ComVisible(true)]
	public class NodeProperties : Microsoft.VisualStudio.Package.LocalizableProperties,
		ISpecifyPropertyPages,
		IVsGetCfgProvider,
		IVsSpecifyProjectDesignerPages,
		EnvDTE80.IInternalExtenderProvider,
		IVsBrowseObject
	{
		#region fields
		private HierarchyNode node;
		#endregion

		#region properties
		[Browsable(false)]
		[AutomationBrowsable(false)]
		public HierarchyNode Node
		{
			get { return this.node; }
		}

		/// <summary>
		/// Used by Property Pages Frame to set it's title bar. The Caption of the Hierarchy Node is returned.
		/// </summary>
		[Browsable(false)]
		[AutomationBrowsable(false)]
		public virtual string Name
		{
			get { return this.node.Caption; }
		}

		#endregion

		#region ctors
		public NodeProperties(HierarchyNode node)
		{
			if (node == null)
			{
				throw new ArgumentNullException("node");
			}
			this.node = node;
		}
		#endregion

		#region ISpecifyPropertyPages methods
		public virtual void GetPages(CAUUID[] pages)
		{
			this.GetCommonPropertyPages(pages);
		}
		#endregion

		#region IVsSpecifyProjectDesignerPages
		/// <summary>
		/// Implementation of the IVsSpecifyProjectDesignerPages. It will retun the pages that are configuration independent.
		/// </summary>
		/// <param name="pages">The pages to return.</param>
		/// <returns></returns>
		public virtual int GetProjectDesignerPages(CAUUID[] pages)
		{
			this.GetCommonPropertyPages(pages);
			return VSConstants.S_OK;
		}
		#endregion

		#region IVsGetCfgProvider methods
		public virtual int GetCfgProvider(out IVsCfgProvider p)
		{
			p = null;
			return VSConstants.E_NOTIMPL;
		}
		#endregion

		#region IVsBrowseObject methods
		/// <summary>
		/// Maps back to the hierarchy or project item object corresponding to the browse object.
		/// </summary>
		/// <param name="hier">Reference to the hierarchy object.</param>
		/// <param name="itemid">Reference to the project item.</param>
		/// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code. </returns>
		public virtual int GetProjectItem(out IVsHierarchy hier, out uint itemid)
		{
			if (this.node == null)
			{
				throw new InvalidOperationException();
			}
			hier = HierarchyNode.GetOuterHierarchy(this.node.ProjectMgr);
			itemid = this.node.ID;
			return VSConstants.S_OK;
		}
		#endregion

		#region overridden methods
		/// <summary>
		/// Get the Caption of the Hierarchy Node instance. If Caption is null or empty we delegate to base
		/// </summary>
		/// <returns>Caption of Hierarchy node instance</returns>
		public override string GetComponentName()
		{
			string caption = this.Node.Caption;
			if (string.IsNullOrEmpty(caption))
			{
				return base.GetComponentName();
			}
			else
			{
				return caption;
			}
		}
		#endregion

		#region helper methods
		protected string GetProperty(string name, string def)
		{
			string a = this.Node.ItemNode.GetMetadata(name);
			return (a == null) ? def : a;
		}

		protected void SetProperty(string name, string value)
		{
			this.Node.ItemNode.SetMetadata(name, value);
		}

		/// <summary>
		/// Retrieves the common property pages. The NodeProperties is the BrowseObject and that will be called to support 
		/// configuration independent properties.
		/// </summary>
		/// <param name="pages">The pages to return.</param>
		private void GetCommonPropertyPages(CAUUID[] pages)
		{
			// We do not check whether the supportsProjectDesigner is set to false on the ProjectNode.
			// We rely that the caller knows what to call on us.
			if (pages == null)
			{
				throw new ArgumentNullException("pages");
			}

			if (pages.Length == 0)
			{
				throw new ArgumentException(SR.GetString(SR.InvalidParameter, CultureInfo.CurrentUICulture), "pages");
			}

			// Only the project should show the property page the rest should show the project properties.
			if (this.node != null && (this.node is ProjectNode))
			{
				// Retrieve the list of guids from hierarchy properties.
				// Because a flavor could modify that list we must make sure we are calling the outer most implementation of IVsHierarchy
				string guidsList = String.Empty;
				IVsHierarchy hierarchy = HierarchyNode.GetOuterHierarchy(this.Node.ProjectMgr);
				object variant = null;
				ErrorHandler.ThrowOnFailure(hierarchy.GetProperty(VSConstants.VSITEMID_ROOT, (int)__VSHPROPID2.VSHPROPID_PropertyPagesCLSIDList, out variant));
				guidsList = (string)variant;

				Guid[] guids = Utilities.GuidsArrayFromSemicolonDelimitedStringOfGuids(guidsList);
				if (guids == null || guids.Length == 0)
				{
					pages[0] = new CAUUID();
					pages[0].cElems = 0;
				}
				else
				{
					pages[0] = PackageUtilities.CreateCAUUIDFromGuidArray(guids);
				}
			}
			else
			{
				pages[0] = new CAUUID();
				pages[0].cElems = 0;
			}
		}
		#endregion

		#region IInternalExtenderProvider Members

		bool EnvDTE80.IInternalExtenderProvider.CanExtend(string extenderCATID, string extenderName, object extendeeObject)
		{
			IVsHierarchy outerHierarchy = HierarchyNode.GetOuterHierarchy(this.Node);
			if (outerHierarchy is EnvDTE80.IInternalExtenderProvider)
			{
				return ((EnvDTE80.IInternalExtenderProvider)outerHierarchy).CanExtend(extenderCATID, extenderName, extendeeObject);
			}
			return false;
		}

		object EnvDTE80.IInternalExtenderProvider.GetExtender(string extenderCATID, string extenderName, object extendeeObject, EnvDTE.IExtenderSite extenderSite, int cookie)
		{
			IVsHierarchy outerHierarchy = HierarchyNode.GetOuterHierarchy(this.Node);
			if (outerHierarchy is EnvDTE80.IInternalExtenderProvider)
			{
				return ((EnvDTE80.IInternalExtenderProvider)outerHierarchy).GetExtender(extenderCATID, extenderName, extendeeObject, extenderSite, cookie);
			}
			return null;
		}

		object EnvDTE80.IInternalExtenderProvider.GetExtenderNames(string extenderCATID, object extendeeObject)
		{
			IVsHierarchy outerHierarchy = HierarchyNode.GetOuterHierarchy(this.Node);
			if (outerHierarchy is EnvDTE80.IInternalExtenderProvider)
			{
				return ((EnvDTE80.IInternalExtenderProvider)outerHierarchy).GetExtenderNames(extenderCATID, extendeeObject);
			}
			return null;
		}

		#endregion

		#region ExtenderSupport
		[Browsable(false)]
		[SuppressMessage("Microsoft.Naming", "CA1709:IdentifiersShouldBeCasedCorrectly", MessageId = "CATID")]
		public virtual string ExtenderCATID
		{
			get
			{
				Guid catid = this.Node.ProjectMgr.GetCATIDForType(this.GetType());
				if (Guid.Empty.CompareTo(catid) == 0)
				{
					return null;
				}
				return catid.ToString("B");
			}
		}

		[Browsable(false)]
		public object ExtenderNames()
		{
			EnvDTE.ObjectExtenders extenderService = (EnvDTE.ObjectExtenders)this.Node.GetService(typeof(EnvDTE.ObjectExtenders));
			Debug.Assert(extenderService != null, "Could not get the ObjectExtenders object from the services exposed by this property object");
			if (extenderService == null)
			{
				throw new InvalidOperationException();
			}
			return extenderService.GetExtenderNames(this.ExtenderCATID, this);
		}

		public object Extender(string extenderName)
		{
			EnvDTE.ObjectExtenders extenderService = (EnvDTE.ObjectExtenders)this.Node.GetService(typeof(EnvDTE.ObjectExtenders));
			Debug.Assert(extenderService != null, "Could not get the ObjectExtenders object from the services exposed by this property object");
			if (extenderService == null)
			{
				throw new InvalidOperationException();
			}
			return extenderService.GetExtender(this.ExtenderCATID, extenderName, this);
		}

		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public abstract class BaseFileNodeProperties : NodeProperties, VSLangProj.FileProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Advanced)]
		[LocDisplayName(SR.BuildAction)]
		[SRDescriptionAttribute(SR.BuildActionDescription)]
		public virtual Microsoft.VisualStudio.Package.BuildAction BuildAction
		{
			get
			{
				string value = this.Node.ItemNode.ItemName;
				if (value == null || value.Length == 0)
				{
					return Microsoft.VisualStudio.Package.BuildAction.None;
				}
				return (Microsoft.VisualStudio.Package.BuildAction)Enum.Parse(typeof(Microsoft.VisualStudio.Package.BuildAction), value);
			}
			set
			{
				this.Node.ItemNode.ItemName = value.ToString();
			}
		}

		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.FullPath)]
		[SRDescriptionAttribute(SR.FullPathDescription)]
		public string FullPath
		{
			get
			{
				return this.Node.Url;
			}
		}

		#region non-browsable properties - used for automation only
		[Browsable(false)]
		public string Extension
		{
			get
			{
				return Path.GetExtension(this.Node.Caption);
			}
		}

		[Browsable(false)]
		public virtual bool IsLink
		{
			get
			{
				return false;
			}
		}

		[Browsable(false)]
		public string URL
		{
			get
			{
				return this.Node.Url;
			}
		}
		#endregion

		#endregion

		#region ctors
		public BaseFileNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion

		#region overridden methods
		public override string GetClassName()
		{
			return SR.GetString(SR.FileProperties, CultureInfo.CurrentUICulture);
		}
		#endregion

		#region VSLangProj.FileProperties Members
		string VSLangProj.FileProperties.Author
		{
			get { return String.Empty; }
		}

		VSLangProj.prjBuildAction VSLangProj.FileProperties.BuildAction
		{
			get
			{
				return (VSLangProj.prjBuildAction)this.BuildAction;
			}
			set
			{
				this.BuildAction = (BuildAction)value;
			}
		}

		string VSLangProj.FileProperties.CustomTool
		{
			get { return String.Empty; }
			set { }
		}

		string VSLangProj.FileProperties.CustomToolNamespace
		{
			get { return String.Empty; }
			set { }
		}

		string VSLangProj.FileProperties.CustomToolOutput
		{
			get { return String.Empty; }
		}

		string VSLangProj.FileProperties.DateCreated
		{
			get { return String.Empty; }
		}

		string VSLangProj.FileProperties.DateModified
		{
			get { return String.Empty; }
		}

		string VSLangProj.FileProperties.ExtenderCATID
		{
			get { return String.Empty; }
		}

		object VSLangProj.FileProperties.ExtenderNames
		{
			get { return String.Empty; }
		}

		string VSLangProj.FileProperties.FileName
		{
			get
			{
				return this.Node.Caption;
			}
			set
			{
				this.SetFileName(value);
			}
		}

		internal virtual void SetFileName(string value)
		{
		}

		uint VSLangProj.FileProperties.Filesize
		{
			get
			{
				FileInfo fileInfo = new FileInfo(this.FullPath);
				return fileInfo.Exists ? (uint)fileInfo.Length : 0;
			}
		}

		string VSLangProj.FileProperties.HTMLTitle
		{
			get { return String.Empty; }
		}

		bool VSLangProj.FileProperties.IsCustomToolOutput
		{
			get { return false; }
		}

		bool VSLangProj.FileProperties.IsDependentFile
		{
			get { return false; }
		}

		bool VSLangProj.FileProperties.IsDesignTimeBuildInput
		{
			get { return false; }
		}

		string VSLangProj.FileProperties.LocalPath
		{
			get { return this.FullPath; }
		}

		string VSLangProj.FileProperties.ModifiedBy
		{
			get { return String.Empty; }
		}

		string VSLangProj.FileProperties.SubType
		{
			get { return String.Empty; }
			set { }
		}

		string VSLangProj.FileProperties.__id
		{
			get { return String.Empty; }
		}

		object VSLangProj.FileProperties.get_Extender(string ExtenderName)
		{
			return null;
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class FileNodeProperties : BaseFileNodeProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.FileName)]
		[SRDescriptionAttribute(SR.FileNameDescription)]
		public string FileName
		{
			get
			{
				return this.Node.Caption;
			}
			set
			{
				this.Node.SetEditLabel(value);
			}
		}
		#endregion

		#region ctors
		public FileNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion

		#region overridden methods
		internal override void SetFileName(string value)
		{
			this.FileName = value;
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class LinkedFileNodeProperties : BaseFileNodeProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.FileName)]
		[SRDescriptionAttribute(SR.FileNameDescription)]
		public string FileName
		{
			get
			{
				return this.Node.Caption;
			}
		}

		[Browsable(false)]
		public override bool IsLink
		{
			get { return true; }
		}
		#endregion

		#region ctors
		public LinkedFileNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class DependentFileNodeProperties : BaseFileNodeProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.FileName)]
		[SRDescriptionAttribute(SR.FileNameDescription)]
		public virtual string FileName
		{
			get
			{
				return this.Node.Caption;
			}
		}
		#endregion

		#region ctors
		public DependentFileNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion

		#region overridden methods
		public override string GetClassName()
		{
			return SR.GetString(SR.FileProperties, CultureInfo.CurrentUICulture);
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class SingleFileGeneratorNodeProperties : FileNodeProperties
	{
		#region fields
		private EventHandler<HierarchyNodeEventArgs> onCustomToolChanged;
		private EventHandler<HierarchyNodeEventArgs> onCustomToolNameSpaceChanged;
		protected string _customTool = "";
		protected string _customToolNamespace = "";
		#endregion

		#region custom tool events
		internal event EventHandler<HierarchyNodeEventArgs> OnCustomToolChanged
		{
			add { onCustomToolChanged += value; }
			remove { onCustomToolChanged -= value; }
		}

		internal event EventHandler<HierarchyNodeEventArgs> OnCustomToolNameSpaceChanged
		{
			add { onCustomToolNameSpaceChanged += value; }
			remove { onCustomToolNameSpaceChanged -= value; }
		}

		#endregion

		#region properties
		[SRCategoryAttribute(SR.Advanced)]
		[LocDisplayName(SR.CustomTool)]
		[SRDescriptionAttribute(SR.CustomToolDescription)]
		public virtual string CustomTool
		{
			get
			{
				_customTool = this.Node.ItemNode.GetMetadata(ProjectFileConstants.Generator);
				return _customTool;
			}
			set
			{
				_customTool = value;
				if (!string.IsNullOrEmpty(_customTool))
				{
					this.Node.ItemNode.SetMetadata(ProjectFileConstants.Generator, _customTool);
					HierarchyNodeEventArgs args = new HierarchyNodeEventArgs(this.Node);
					if (onCustomToolChanged != null)
					{
						onCustomToolChanged(this.Node, args);
					}
				}
			}
		}

		[SRCategoryAttribute(Microsoft.VisualStudio.Package.SR.Advanced)]
		[LocDisplayName(SR.CustomToolNamespace)]
		[SRDescriptionAttribute(SR.CustomToolNamespaceDescription)]
		public virtual string CustomToolNamespace
		{
			get
			{
				_customToolNamespace = this.Node.ItemNode.GetMetadata(ProjectFileConstants.CustomToolNamespace);
				return _customToolNamespace;
			}
			set
			{
				_customToolNamespace = value;
				if (!string.IsNullOrEmpty(_customToolNamespace))
				{
					this.Node.ItemNode.SetMetadata(ProjectFileConstants.CustomToolNamespace, _customToolNamespace);
					HierarchyNodeEventArgs args = new HierarchyNodeEventArgs(this.Node);
					if (onCustomToolNameSpaceChanged != null)
					{
						onCustomToolNameSpaceChanged(this.Node, args);
					}
				}
			}
		}
		#endregion

		#region ctors
		public SingleFileGeneratorNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class ProjectNodeProperties : NodeProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.ProjectFolder)]
		[SRDescriptionAttribute(SR.ProjectFolderDescription)]
		[AutomationBrowsable(false)]
		public string ProjectFolder
		{
			get
			{
				return this.Node.ProjectMgr.ProjectFolder;
			}
		}

		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.ProjectFile)]
		[SRDescriptionAttribute(SR.ProjectFileDescription)]
		[AutomationBrowsable(false)]
		public string ProjectFile
		{
			get
			{
				return this.Node.ProjectMgr.ProjectFile;
			}
			set
			{
				this.Node.ProjectMgr.ProjectFile = value;
			}
		}

		#region non-browsable properties - used for automation only
		[Browsable(false)]
		public string FileName
		{
			get
			{
				return this.Node.ProjectMgr.ProjectFile;
			}
			set
			{
				this.Node.ProjectMgr.ProjectFile = value;
			}
		}


		[Browsable(false)]
		public string FullPath
		{
			get
			{
				string fullPath = this.Node.ProjectMgr.ProjectFolder;
				if (!fullPath.EndsWith(Path.DirectorySeparatorChar.ToString(), StringComparison.Ordinal))
				{
					return fullPath + Path.DirectorySeparatorChar;
				}
				else
				{
					return fullPath;
				}
			}
		}
		#endregion

		#endregion

		#region ctors
		public ProjectNodeProperties(ProjectNode node)
			: base(node)
		{
		}
		#endregion

		#region overridden methods
		public override string GetClassName()
		{
			return SR.GetString(SR.ProjectProperties, CultureInfo.CurrentUICulture);
		}

		/// <summary>
		/// ICustomTypeDescriptor.GetEditor
		/// To enable the "Property Pages" button on the properties browser
		/// the browse object (project properties) need to be unmanaged
		/// or it needs to provide an editor of type ComponentEditor.
		/// </summary>
		/// <param name="editorBaseType">Type of the editor</param>
		/// <returns>Editor</returns>
		public override object GetEditor(Type editorBaseType)
		{
			// Override the scenario where we are asked for a ComponentEditor
			// as this is how the Properties Browser calls us
			if (Utilities.AreTypesEquivalent(editorBaseType, typeof(ComponentEditor)))
			{
				IOleServiceProvider sp;
				ErrorHandler.ThrowOnFailure(this.Node.GetSite(out sp));
				return new PropertiesEditorLauncher(new ServiceProvider(sp));
			}

			return base.GetEditor(editorBaseType);
		}

		public override int GetCfgProvider(out IVsCfgProvider p)
		{
			if (this.Node != null && this.Node.ProjectMgr != null)
			{
				return this.Node.ProjectMgr.GetCfgProvider(out p);
			}

			return base.GetCfgProvider(out p);
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class FolderNodeProperties : NodeProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.FolderName)]
		[SRDescriptionAttribute(SR.FolderNameDescription)]
		[AutomationBrowsable(false)]
		public string FolderName
		{
			get
			{
				return this.Node.Caption;
			}
			set
			{
				this.Node.SetEditLabel(value);
				this.Node.ReDraw(UIHierarchyElement.Caption);
			}
		}

		#region properties - used for automation only
		[Browsable(false)]
		[AutomationBrowsable(true)]
		public string FileName
		{
			get
			{
				return this.Node.Caption;
			}
			set
			{
				this.Node.SetEditLabel(value);
			}
		}

		[Browsable(false)]
		[AutomationBrowsable(true)]
		public string FullPath
		{
			get
			{
				string fullPath = this.Node.GetMkDocument();
				if (!fullPath.EndsWith(Path.DirectorySeparatorChar.ToString(), StringComparison.Ordinal))
				{
					return fullPath + Path.DirectorySeparatorChar;
				}
				else
				{
					return fullPath;
				}
			}
		}
		#endregion

		#endregion

		#region ctors
		public FolderNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion

		#region overridden methods
		public override string GetClassName()
		{
			return SR.GetString(SR.FolderProperties, CultureInfo.CurrentUICulture);
		}
		#endregion
	}

	[CLSCompliant(false), ComVisible(true)]
	public class ReferenceNodeProperties : NodeProperties
	{
		#region properties
		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.RefName)]
		[SRDescriptionAttribute(SR.RefNameDescription)]
		[Browsable(true)]
		[AutomationBrowsable(true)]
		public override string Name
		{
			get
			{
				return this.Node.Caption;
			}
		}

		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.CopyToLocal)]
		[SRDescriptionAttribute(SR.CopyToLocalDescription)]
		public virtual bool CopyToLocal
		{
			get
			{
				string copyLocal = this.GetProperty(ProjectFileConstants.Private, "False");
				if (copyLocal == null || copyLocal.Length == 0)
					return true;
				return bool.Parse(copyLocal);
			}
			set
			{
				this.SetProperty(ProjectFileConstants.Private, value.ToString());
			}
		}

		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.FullPath)]
		[SRDescriptionAttribute(SR.FullPathDescription)]
		public virtual string FullPath
		{
			get
			{
				return this.Node.Url;
			}
		}
		#endregion

		#region ctors
		public ReferenceNodeProperties(HierarchyNode node)
			: base(node)
		{
		}
		#endregion

		#region overridden methods
		public override string GetClassName()
		{
			return SR.GetString(SR.ReferenceProperties, CultureInfo.CurrentUICulture);
		}
		#endregion
	}

	[ComVisible(true)]
	public class ProjectReferencesProperties : ReferenceNodeProperties
	{
		#region ctors
		public ProjectReferencesProperties(ProjectReferenceNode node)
			: base(node)
		{
		}
		#endregion

		#region properties
		/// <summary>
		/// Overrides Name to not be displayed. ReferenceName is to be used
		/// instead.
		/// </summary>
		[Browsable(false)]
		public override string Name
		{
			get
			{
				return this.ReferenceName;
			}
		}

		[SRCategoryAttribute(SR.Misc)]
		[LocDisplayName(SR.RefName)]
		[SRDescriptionAttribute(SR.RefNameDescription)]
		[Browsable(true)]
		[AutomationBrowsable(true)]
		public string ReferenceName
		{
			get
			{
				return this.Node.Caption;
			}
			set
			{
				this.Node.SetEditLabel(value);
			}
		}
		#endregion

		#region overriden methods
		public override string FullPath
		{
			get
			{
				return ((ProjectReferenceNode)Node).ReferencedProjectOutputPath;
			}
		}
		#endregion
	}
}
