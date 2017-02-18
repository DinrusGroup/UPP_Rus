#ifndef _XMLCommand_h_
#define _XMLCommand_h_

#include <CtrlCore/CtrlCore.h>

NAMESPACE_UPP

////////////////////////////////////////////////////////////////////////////////////
// a single command, i.e. available entry for menu and bars
class XMLCommand
{
	friend class XMLCommands;
	private:
		// embedded control, if any
		Ptr<Ctrl> control;
		Size ctrlSize;
		
		// associated callback, if any
		Callback callback;
		
		// enabled flag
		bool enabled;
		
		// custom command flag
		bool custom;

	public:
		Ctrl *GetCtrl(void) const				{ return control;	}
		Size const &GetCtrlSize(void) const		{ return ctrlSize;	}
		Callback const &GetCallback(void) const	{ return callback;	}
		bool GetIsEnabled(void) const			{ return enabled;	}
		bool GetIsCustom(void) const			{ return custom;	}
		
		bool operator==(XMLCommand &other) const;
		
		// xml support
		void Xmlize(XmlIO xml);
};

////////////////////////////////////////////////////////////////////////////////////
// an array of available commands
class XMLCommands : DeepCopyOption<XMLCommands>
{
	private:
		ArrayMap<String, XMLCommand> commands;

	public:
		// default constructor
		XMLCommands() {}
		
		// copy constructor
		XMLCommands(XMLCommands const &cmds, int dummy) : commands(cmds.commands, 0) {}
		
		// pick constructor
		XMLCommands(XMLCommands pick_ &cmds) : commands(cmds.commands) {}
			
		// adds a custom command
		XMLCommands &Add(String const &id);
		
		// adds a built-in command with given callback
		XMLCommands &Add(String const &id, Callback cb);
		
		// adds a control
		XMLCommands &Add(String const &id, Ctrl &ctrl);
		XMLCommands &Add(String const &id, Ctrl &ctrl, Size const &size);
		
		// adds a custom command, allows enable/disable item
		XMLCommands &Add(bool enabled, String const &id);
		
		// adds a built-in command with given callback, allows enable/disable item
		XMLCommands &Add(bool enabled, String const &id, Callback cb);
		
		// adds a control, allows enable/disable item
		XMLCommands &Add(bool enabled, String const &id, Ctrl &ctrl);
		XMLCommands &Add(bool enabled, String const &id, Ctrl &ctrl, Size const &size);
		
		// removes a command
		XMLCommands &Remove(int idx) { commands.Remove(idx); return *this; }

		// get all available command IDs
		Vector<String> const &GetIds(void) const;
		
		// get a command for a given id
		XMLCommand const &Get(String const &id) const { return commands.Get(id); }
		
		// sets the commands by a callback
		void Set(Callback1<XMLCommands &> commands);
		
		// checks wether a command is present given its id
		bool Has(String const &id) const { return commands.Find(id) >= 0; }
		
		// array access
		XMLCommand const &operator[](int idx) const { return commands[idx]; }
		XMLCommand &operator[](int idx) { return commands[idx]; }
		
		// sort items - alphabetically, but first built-in commands, then custom ones
		XMLCommands &Sort(void);
		
		// xml support
		void Xmlize(XmlIO xml);
};

END_UPP_NAMESPACE

#endif