/*** DOCUMENTATION
	<application name="StateCtl" language="en_US">
		<synopsis>
			StateCtl conference bridge.
		</synopsis>
		<syntax>
			<parameter name="confno">
				<para>The conference number</para>
			</parameter>
			<parameter name="options">
				<optionlist>
					<option name="a">
						<para>Set admin mode.</para>
					</option>
					<option name="A">
						<para>Set marked mode.</para>
					</option>
					<option name="b">
						<para>Run AGI script specified in <variable>STATECTL_AGI_BACKGROUND</variable>
						Default: <literal>conf-background.agi</literal>.</para>
						<note><para>This does not work with non-DAHDI channels in the same
						conference).</para></note>
					</option>
					<option name="c">
						<para>Announce user(s) count on joining a conference.</para>
					</option>
					<option name="C">
						<para>Continue in dialplan when kicked out of conference.</para>
					</option>
					<option name="d">
						<para>Dynamically add conference.</para>
					</option>
					<option name="D">
						<para>Dynamically add conference, prompting for a PIN.</para>
					</option>
					<option name="e">
						<para>Select an empty conference.</para>
					</option>
					<option name="E">
						<para>Select an empty pinless conference.</para>
					</option>
					<option name="F">
						<para>Pass DTMF through the conference.</para>
					</option>
					<option name="G">
						<argument name="x" required="true">
							<para>The file to playback</para>
						</argument>
						<para>Play an intro announcement in conference.</para>
					</option>
					<option name="i">
						<para>Announce user join/leave with review.</para>
					</option>
					<option name="I">
						<para>Announce user join/leave without review.</para>
					</option>
					<option name="k">
						<para>Close the conference if there's only one active participant left at exit.</para>
					</option>
					<option name="l">
						<para>Set listen only mode (Listen only, no talking).</para>
					</option>
					<option name="m">
						<para>Set initially muted.</para>
					</option>
					<option name="M" hasparams="optional">
						<para>Enable music on hold when the conference has a single caller. Optionally,
						specify a musiconhold class to use. If one is not provided, it will use the
						channel's currently set music class, or <literal>default</literal>.</para>
						<argument name="class" required="true" />
					</option>
					<option name="n">
						<para>Disable the denoiser. By default, if <literal>func_speex</literal> is loaded, Asterisk
						will apply a denoiser to channels in the StateCtl conference. However, channel
						drivers that present audio with a varying rate will experience degraded
						performance with a denoiser attached. This parameter allows a channel joining
						the conference to choose not to have a denoiser attached without having to
						unload <literal>func_speex</literal>.</para>
					</option>
					<option name="o">
						<para>Set talker optimization - treats talkers who aren't speaking as
						being muted, meaning (a) No encode is done on transmission and (b)
						Received audio that is not registered as talking is omitted causing no
						buildup in background noise.</para>
					</option>
					<option name="p" hasparams="optional">
						<para>Allow user to exit the conference by pressing <literal>#</literal> (default)
						or any of the defined keys. Dial plan execution will continue at the next
						priority following StateCtl. The key used is set to channel variable
						<variable>STATECTL_EXIT_KEY</variable>.</para>
						<argument name="keys" required="true" />
						<note>
							<para>Option <literal>s</literal> has priority for <literal>*</literal>
							since it cannot change its activation code.</para>
						</note>
					</option>
					<option name="P">
						<para>Always prompt for the pin even if it is specified.</para>
					</option>
					<option name="q">
						<para>Quiet mode (don't play enter/leave sounds).</para>
					</option>
					<option name="r">
						<para>Record conference (records as <variable>STATECTL_RECORDINGFILE</variable>
						using format <variable>STATECTL_RECORDINGFORMAT</variable>. Default filename is
						<literal>statectl-conf-rec-${CONFNO}-${UNIQUEID}</literal> and the default format is
						wav.</para>
					</option>
					<option name="s">
						<para>Present menu (user or admin) when <literal>*</literal> is received
						(send to menu).</para>
					</option>
					<option name="t">
						<para>Set talk only mode. (Talk only, no listening).</para>
					</option>
					<option name="T">
						<para>Set talker detection (sent to manager interface and statectl list).</para>
					</option>
					<option name="v" hasparams="optional">
						<para>Announce when a user is joining or leaving the conference.  Use the voicemail greeting as the announcement.
						 If the i or I options are set, the application will fall back to them if no voicemail greeting can be found.</para>
						<argument name="mailbox@[context]" required="true">
							<para>The mailbox and voicemail context to play from.  If no context provided, assumed context is default.</para>
						</argument>
					</option>
					<option name="w" hasparams="optional">
						<para>Wait until the marked user enters the conference.</para>
						<argument name="secs" required="true" />
					</option>
					<option name="x">
						<para>Leave the conference when the last marked user leaves.</para>
					</option>
					<option name="X">
						<para>Allow user to exit the conference by entering a valid single digit
						extension <variable>STATECTL_EXIT_CONTEXT</variable> or the current context
						if that variable is not defined.</para>
						<note>
							<para>Option <literal>s</literal> has priority for <literal>*</literal>
							since it cannot change its activation code.</para>
						</note>
					</option>
					<option name="1">
						<para>Do not play message when first person enters</para>
					</option>
					<option name="S">
						<para>Kick the user <replaceable>x</replaceable> seconds <emphasis>after</emphasis> he entered into
						the conference.</para>
						<argument name="x" required="true" />
					</option>
					<option name="L" argsep=":">
						<para>Limit the conference to <replaceable>x</replaceable> ms. Play a warning when
						<replaceable>y</replaceable> ms are left. Repeat the warning every <replaceable>z</replaceable> ms.
						The following special variables can be used with this option:</para>
						<variablelist>
							<variable name="CONF_LIMIT_TIMEOUT_FILE">
								<para>File to play when time is up.</para>
							</variable>
							<variable name="CONF_LIMIT_WARNING_FILE">
								<para>File to play as warning if <replaceable>y</replaceable> is defined. The
								default is to say the time remaining.</para>
							</variable>
						</variablelist>
						<argument name="x" />
						<argument name="y" />
						<argument name="z" />
					</option>
				</optionlist>
			</parameter>
			<parameter name="pin" />
		</syntax>
		<description>
			<para>Enters the user into a specified StateCtl conference.  If the <replaceable>confno</replaceable>
			is omitted, the user will be prompted to enter one.  User can exit the conference by hangup, or
			if the <literal>p</literal> option is specified, by pressing <literal>#</literal>.</para>
			<note><para>The DAHDI kernel modules and a functional DAHDI timing source (see dahdi_test)
			must be present for conferencing to operate properly. In addition, the chan_dahdi channel driver
			must be loaded for the <literal>i</literal> and <literal>r</literal> options to operate at
			all.</para></note>
		</description>
		<see-also>
			<ref type="application">StateCtlCount</ref>
			<ref type="application">StateCtlAdmin</ref>
			<ref type="application">StateCtlChannelAdmin</ref>
		</see-also>
	</application>
	<application name="StateCtlCount" language="en_US">
		<synopsis>
			StateCtl participant count.
		</synopsis>
		<syntax>
			<parameter name="confno" required="true">
				<para>Conference number.</para>
			</parameter>
			<parameter name="var" />
		</syntax>
		<description>
			<para>Plays back the number of users in the specified StateCtl conference.
			If <replaceable>var</replaceable> is specified, playback will be skipped and the value
			will be returned in the variable. Upon application completion, StateCtlCount will hangup
			the channel, unless priority <literal>n+1</literal> exists, in which case priority progress will
			continue.</para>
		</description>
		<see-also>
			<ref type="application">StateCtl</ref>
		</see-also>
	</application>
	<application name="StateCtlAdmin" language="en_US">
		<synopsis>
			StateCtl conference administration.
		</synopsis>
		<syntax>
			<parameter name="confno" required="true" />
			<parameter name="command" required="true">
				<optionlist>
					<option name="e">
						<para>Eject last user that joined.</para>
					</option>
					<option name="E">
						<para>Extend conference end time, if scheduled.</para>
					</option>
					<option name="k">
						<para>Kick one user out of conference.</para>
					</option>
					<option name="K">
						<para>Kick all users out of conference.</para>
					</option>
					<option name="l">
						<para>Unlock conference.</para>
					</option>
					<option name="L">
						<para>Lock conference.</para>
					</option>
					<option name="m">
						<para>Unmute one user.</para>
					</option>
					<option name="M">
						<para>Mute one user.</para>
					</option>
					<option name="n">
						<para>Unmute all users in the conference.</para>
					</option>
					<option name="N">
						<para>Mute all non-admin users in the conference.</para>
					</option>
					<option name="r">
						<para>Reset one user's volume settings.</para>
					</option>
					<option name="R">
						<para>Reset all users volume settings.</para>
					</option>
					<option name="s">
						<para>Lower entire conference speaking volume.</para>
					</option>
					<option name="S">
						<para>Raise entire conference speaking volume.</para>
					</option>
					<option name="t">
						<para>Lower one user's talk volume.</para>
					</option>
					<option name="T">
						<para>Raise one user's talk volume.</para>
					</option>
					<option name="u">
						<para>Lower one user's listen volume.</para>
					</option>
					<option name="U">
						<para>Raise one user's listen volume.</para>
					</option>
					<option name="v">
						<para>Lower entire conference listening volume.</para>
					</option>
					<option name="V">
						<para>Raise entire conference listening volume.</para>
					</option>
				</optionlist>
			</parameter>
			<parameter name="user" />
		</syntax>
		<description>
			<para>Run admin <replaceable>command</replaceable> for conference <replaceable>confno</replaceable>.</para>
			<para>Will additionally set the variable <variable>STATECTLADMINSTATUS</variable> with one of
			the following values:</para>
			<variablelist>
				<variable name="STATECTLADMINSTATUS">
					<value name="NOPARSE">
						Invalid arguments.
					</value>
					<value name="NOTFOUND">
						User specified was not found.
					</value>
					<value name="FAILED">
						Another failure occurred.
					</value>
					<value name="OK">
						The operation was completed successfully.
					</value>
				</variable>
			</variablelist>
		</description>
		<see-also>
			<ref type="application">StateCtl</ref>
		</see-also>
	</application>
	<application name="StateCtlChannelAdmin" language="en_US">
		<synopsis>
			StateCtl conference Administration (channel specific).
		</synopsis>
		<syntax>
			<parameter name="channel" required="true" />
			<parameter name="command" required="true">
				<optionlist>
					<option name="k">
						<para>Kick the specified user out of the conference he is in.</para>
					</option>
					<option name="m">
						<para>Unmute the specified user.</para>
					</option>
					<option name="M">
						<para>Mute the specified user.</para>
					</option>
				</optionlist>
			</parameter>
		</syntax>
		<description>
			<para>Run admin <replaceable>command</replaceable> for a specific
			<replaceable>channel</replaceable> in any conference.</para>
		</description>
	</application>
	<application name="SLAStation" language="en_US">
		<synopsis>
			Shared Line Appearance Station.
		</synopsis>
		<syntax>
			<parameter name="station" required="true">
				<para>Station name</para>
			</parameter>
		</syntax>
		<description>
			<para>This application should be executed by an SLA station. The argument depends
			on how the call was initiated. If the phone was just taken off hook, then the argument
			<replaceable>station</replaceable> should be just the station name. If the call was
			initiated by pressing a line key, then the station name should be preceded by an underscore
			and the trunk name associated with that line button.</para>
			<para>For example: <literal>station1_line1</literal></para>
			<para>On exit, this application will set the variable <variable>SLASTATION_STATUS</variable> to
			one of the following values:</para>
			<variablelist>
				<variable name="SLASTATION_STATUS">
					<value name="FAILURE" />
					<value name="CONGESTION" />
					<value name="SUCCESS" />
				</variable>
			</variablelist>
		</description>
	</application>
	<application name="SLATrunk" language="en_US">
		<synopsis>
			Shared Line Appearance Trunk.
		</synopsis>
		<syntax>
			<parameter name="trunk" required="true">
				<para>Trunk name</para>
			</parameter>
			<parameter name="options">
				<optionlist>
					<option name="M" hasparams="optional">
						<para>Play back the specified MOH <replaceable>class</replaceable>
						instead of ringing</para>
						<argument name="class" required="true" />
					</option>
				</optionlist>
			</parameter>
		</syntax>
		<description>
			<para>This application should be executed by an SLA trunk on an inbound call. The channel calling
			this application should correspond to the SLA trunk with the name <replaceable>trunk</replaceable>
			that is being passed as an argument.</para>
			<para>On exit, this application will set the variable <variable>SLATRUNK_STATUS</variable> to
			one of the following values:</para>
			<variablelist>
				<variable name="SLATRUNK_STATUS">
					<value name="FAILURE" />
					<value name="SUCCESS" />
					<value name="UNANSWERED" />
					<value name="RINGTIMEOUT" />
				</variable>
			</variablelist>
		</description>
	</application>
	<function name="STATECTL_INFO" language="en_US">
		<synopsis>
			Query a given conference of various properties.
		</synopsis>
		<syntax>
			<parameter name="keyword" required="true">
				<para>Options:</para>
				<enumlist>
					<enum name="lock">
						<para>Boolean of whether the corresponding conference is locked.</para>
					</enum>
					<enum name="parties">
						<para>Number of parties in a given conference</para>
					</enum>
					<enum name="activity">
						<para>Duration of conference in seconds.</para>
					</enum>
					<enum name="dynamic">
						<para>Boolean of whether the corresponding conference is dynamic.</para>
					</enum>
				</enumlist>
			</parameter>
			<parameter name="confno" required="true">
				<para>Conference number to retrieve information from.</para>
			</parameter>
		</syntax>
		<description />
		<see-also>
			<ref type="application">StateCtl</ref>
			<ref type="application">StateCtlCount</ref>
			<ref type="application">StateCtlAdmin</ref>
			<ref type="application">StateCtlChannelAdmin</ref>
		</see-also>
	</function>
	<manager name="StatectlMute" language="en_US">
		<synopsis>
			Mute a Statectl user.
		</synopsis>
		<syntax>
			<xi:include xpointer="xpointer(/docs/manager[@name='Login']/syntax/parameter[@name='ActionID'])" />
			<parameter name="Statectl" required="true" />
			<parameter name="Usernum" required="true" />
		</syntax>
		<description>
		</description>
	</manager>
	<manager name="StatectlUnmute" language="en_US">
		<synopsis>
			Unmute a Statectl user.
		</synopsis>
		<syntax>
			<xi:include xpointer="xpointer(/docs/manager[@name='Login']/syntax/parameter[@name='ActionID'])" />
			<parameter name="Statectl" required="true" />
			<parameter name="Usernum" required="true" />
		</syntax>
		<description>
		</description>
	</manager>
	<manager name="StatectlList" language="en_US">
		<synopsis>
			List participants in a conference.
		</synopsis>
		<syntax>
			<xi:include xpointer="xpointer(/docs/manager[@name='Login']/syntax/parameter[@name='ActionID'])" />
			<parameter name="Conference" required="false">
				<para>Conference number.</para>
			</parameter>
		</syntax>
		<description>
			<para>Lists all users in a particular StateCtl conference.
			StatectlList will follow as separate events, followed by a final event called
			StatectlListComplete.</para>
		</description>
	</manager>
	<manager name="StatectlListRooms" language="en_US">
		<synopsis>
			List active conferences.
		</synopsis>
		<syntax>
			<xi:include xpointer="xpointer(/docs/manager[@name='Login']/syntax/parameter[@name='ActionID'])" />
		</syntax>
		<description>
			<para>Lists data about all active conferences.
				StatectlListRooms will follow as separate events, followed by a final event called
				StatectlListRoomsComplete.</para>
		</description>
	</manager>
 ***/

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 114167 $")

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/config.h"
#include "asterisk/app.h"
#include "asterisk/dsp.h"
#include "asterisk/musiconhold.h"
#include "asterisk/manager.h"
#include "asterisk/cli.h"
#include "asterisk/say.h"
#include "asterisk/utils.h"
#include "asterisk/translate.h"
#include "asterisk/ulaw.h"
#include "asterisk/devicestate.h"
#include "asterisk/dial.h"
#include "asterisk/causes.h"
#include "asterisk/paths.h"

#include "asterisk/md5.h"

#include "enter.h"
#include "leave.h"

#define CONFIG_FILE_NAME "statectl.conf"
#define STR_CONCISE			"concise"

enum {
	ADMINFLAG_MUTED =     (1 << 1), /*!< User is muted */
	ADMINFLAG_SELFMUTED = (1 << 2), /*!< User muted self */
	ADMINFLAG_KICKME =    (1 << 3)  /*!< User has been kicked */
};

#define STATECTL_DELAYDETECTTALK     300
#define STATECTL_DELAYDETECTENDTALK  1000
#define AST_FRAME_BITS  32

enum volume_action {
	VOL_UP,
	VOL_DOWN
};

enum entrance_sound {
	ENTER,
	LEAVE
};

enum recording_state {
	STATECTL_RECORD_OFF,
	STATECTL_RECORD_STARTED,
	STATECTL_RECORD_ACTIVE,
	STATECTL_RECORD_TERMINATE
};

#define CONF_SIZE  320

enum {
	/*! user has admin access on the conference */
	CONFFLAG_ADMIN = (1 << 0),
	/*! If set the user can only receive audio from the conference */
	CONFFLAG_MONITOR = (1 << 1),
	/*! If set asterisk will exit conference when '#' is pressed */
	CONFFLAG_POUNDEXIT = (1 << 2),
	/*! If set asterisk will provide a menu to the user when '*' is pressed */
	CONFFLAG_STARMENU = (1 << 3),
	/*! If set the use can only send audio to the conference */
	CONFFLAG_TALKER = (1 << 4),
	/*! If set there will be no enter or leave sounds */
	CONFFLAG_QUIET = (1 << 5),
	/*! If set, when user joins the conference, they will be told the number 
	 *  of users that are already in */
	CONFFLAG_ANNOUNCEUSERCOUNT = (1 << 6),
	/*! Set to run AGI Script in Background */
	CONFFLAG_AGI = (1 << 7),
	/*! Set to have music on hold when user is alone in conference */
	CONFFLAG_MOH = (1 << 8),
	/*! If set the StateCtl will return if all marked with this flag left */
	CONFFLAG_MARKEDEXIT = (1 << 9),
	/*! If set, the StateCtl will wait until a marked user enters */
	CONFFLAG_WAITMARKED = (1 << 10),
	/*! If set, the StateCtl will exit to the specified context */
	CONFFLAG_EXIT_CONTEXT = (1 << 11),
	/*! If set, the user will be marked */
	CONFFLAG_MARKEDUSER = (1 << 12),
	/*! If set, user will be ask record name on entry of conference */
	CONFFLAG_INTROUSER = (1 << 13),
	/*! If set, the StateCtl will be recorded */
	CONFFLAG_RECORDCONF = (1<< 14),
	/*! If set, the user will be monitored if the user is talking or not */
	CONFFLAG_MONITORTALKER = (1 << 15),
	CONFFLAG_DYNAMIC = (1 << 16),
	CONFFLAG_DYNAMICPIN = (1 << 17),
	CONFFLAG_EMPTY = (1 << 18),
	CONFFLAG_EMPTYNOPIN = (1 << 19),
	CONFFLAG_ALWAYSPROMPT = (1 << 20),
	/*! If set, treats talking users as muted users */
	CONFFLAG_OPTIMIZETALKER = (1 << 21),
	/*! If set, won't speak the extra prompt when the first person 
	 *  enters the conference */
	CONFFLAG_NOONLYPERSON = (1 << 22),
	/*! If set, user will be asked to record name on entry of conference 
	 *  without review */
	CONFFLAG_INTROUSERNOREVIEW = (1 << 23),
	/*! If set, the user will be initially self-muted */
	CONFFLAG_STARTMUTED = (1 << 24),
	/*! Pass DTMF through the conference */
	CONFFLAG_PASS_DTMF = (1 << 25),
	/*! This is a SLA station. (Only for use by the SLA applications.) */
	CONFFLAG_SLA_STATION = (1 << 26),
	/*! This is a SLA trunk. (Only for use by the SLA applications.) */
	CONFFLAG_SLA_TRUNK = (1 << 27),
};

enum {
	OPT_ARG_WAITMARKED = 0,
	OPT_ARG_ARRAY_SIZE = 1,
};

AST_APP_OPTIONS(statectl_opts, BEGIN_OPTIONS
	AST_APP_OPTION('A', CONFFLAG_MARKEDUSER ),
	AST_APP_OPTION('a', CONFFLAG_ADMIN ),
	AST_APP_OPTION('b', CONFFLAG_AGI ),
	AST_APP_OPTION('c', CONFFLAG_ANNOUNCEUSERCOUNT ),
	AST_APP_OPTION('D', CONFFLAG_DYNAMICPIN ),
	AST_APP_OPTION('d', CONFFLAG_DYNAMIC ),
	AST_APP_OPTION('E', CONFFLAG_EMPTYNOPIN ),
	AST_APP_OPTION('e', CONFFLAG_EMPTY ),
	AST_APP_OPTION('F', CONFFLAG_PASS_DTMF ),
	AST_APP_OPTION('i', CONFFLAG_INTROUSER ),
	AST_APP_OPTION('I', CONFFLAG_INTROUSERNOREVIEW ),
	AST_APP_OPTION('M', CONFFLAG_MOH ),
	AST_APP_OPTION('m', CONFFLAG_STARTMUTED ),
	AST_APP_OPTION('o', CONFFLAG_OPTIMIZETALKER ),
	AST_APP_OPTION('P', CONFFLAG_ALWAYSPROMPT ),
	AST_APP_OPTION('p', CONFFLAG_POUNDEXIT ),
	AST_APP_OPTION('q', CONFFLAG_QUIET ),
	AST_APP_OPTION('r', CONFFLAG_RECORDCONF ),
	AST_APP_OPTION('s', CONFFLAG_STARMENU ),
	AST_APP_OPTION('T', CONFFLAG_MONITORTALKER ),
	AST_APP_OPTION('l', CONFFLAG_MONITOR ),
	AST_APP_OPTION('t', CONFFLAG_TALKER ),
	AST_APP_OPTION_ARG('w', CONFFLAG_WAITMARKED, OPT_ARG_WAITMARKED ),
	AST_APP_OPTION('X', CONFFLAG_EXIT_CONTEXT ),
	AST_APP_OPTION('x', CONFFLAG_MARKEDEXIT ),
	AST_APP_OPTION('1', CONFFLAG_NOONLYPERSON ),
END_OPTIONS );

static const char * const app = "StateCtl";

#define MAX_CONFNUM 80
#define MAX_PIN     80

static int query_interval=60;


/*! \brief The StateCtl Conference object */
struct ast_conference {
	ast_mutex_t playlock;                   /*!< Conference specific lock (players) */
	ast_mutex_t listenlock;                 /*!< Conference specific lock (listeners) */
	char confno[MAX_CONFNUM];               /*!< Conference */
	struct ast_channel *chan;               /*!< Announcements channel */
	struct ast_channel *lchan;              /*!< Listen/Record channel */
	int fd;                                 /*!< Announcements fd */
	int zapconf;                            /*!< Zaptel Conf # */
	int users;                              /*!< Number of active users */
	int markedusers;                        /*!< Number of marked users */
	int user_count;
	time_t start;                           /*!< Start time (s) */
	int refcount;                           /*!< reference count of usage */
	enum recording_state recording:2;       /*!< recording status */
	unsigned int isdynamic:1;               /*!< Created on the fly? */
	unsigned int locked:1;                  /*!< Is the conference locked? */
	pthread_t recordthread,soundthread;                 /*!< thread for recording */
	ast_mutex_t recordthreadlock,soundthreadlock;		/*!< control threads trying to start recordthread */
	pthread_attr_t attr;                    /*!< thread attribute */
	const char *recordingfilename;          /*!< Filename to record the Conference into */
	const char *recordingformat;            /*!< Format to record the Conference in */
	char pin[MAX_PIN];                      /*!< If protected by a PIN */
	char pinadmin[MAX_PIN];                 /*!< If protected by a admin PIN */
	char maxcount[10];                      //anita-statectl max users count
	struct ast_frame *transframe[32];
	struct ast_frame *origframe;
	struct ast_trans_pvt *transpath[32];

	struct sockaddr_in MixingServerIP;    //address of MixingServer
	char StatectlSession[64];         //the unique session of one Statectl Room
	
	int auth_mode;                //1-white list 2-max member 3-auth password
	int business_mode;            //1-ptt 2-audio 3-video
	struct sockaddr_in pttip;
	struct sockaddr_in pttip_tmp;		
	
	char pttid[80];

	AST_LIST_HEAD_NOLOCK(, ast_conf_user) userlist;
	AST_LIST_ENTRY(ast_conference) list;
};

static AST_LIST_HEAD_STATIC(confs, ast_conference);

static unsigned int conf_map[1024] = {0, };

struct volume {
	int desired;                            /*!< Desired volume adjustment */
	int actual;                             /*!< Actual volume adjustment (for channels that can't adjust) */
};


enum {
	MUTED =     (1 << 1), /*!< User is muted */
	HOLDED =   (1 << 3),  /*!< User is hold */
};

struct ast_conf_user {
	int user_status;
	int mute_status;/*mute flag: 1, mute*/
	int hold_status;/*hold flag: 1,hold*/
	char cid_num[50];                      /*!< Custom User Value */
	char cid_name[50];                      /*!< Custom User Value */
	int user_no;                            /*!< User Number */
	struct ast_flags64 userflags;           /*!< Flags as set in the conference */
	int adminflags;                         /*!< Flags set by the Admin */
	struct ast_channel *chan;               /*!< Connected channel */
	int talking;                            /*!< Is user talking */
	int zapchannel;                         /*!< Is a Zaptel channel */
	char usrvalue[50];                      /*!< Custom User Value */
	char namerecloc[PATH_MAX];				/*!< Name Recorded file Location */
	time_t jointime;                        /*!< Time the user joined the conference */
	struct volume talk;
	struct volume listen;
	AST_LIST_ENTRY(ast_conf_user) list;
};


/*! Map 'volume' levels from -5 through +5 into
 *  decibel (dB) settings for channel drivers
 *  Note: these are not a straight linear-to-dB
 *  conversion... the numbers have been modified
 *  to give the user a better level of adjustability
 */
static char const gain_map[] = {
	-15,
	-13,
	-10,
	-6,
	0,
	0,
	0,
	6,
	10,
	13,
	15,
};

static char prompt_sounds_path[256] = "/var/lib/asterisk/sounds";

static char Mixing_media_ip[30]= {0};
static int Mixing_media_port = 0;

struct mixing_resp{
	char action[6+1];
	char id[32+1];
	struct sockaddr_in addr;
};

static void generate_statectl_uniqueid(const char *cnfno, char *out_uniqueid){
    struct timeval now;
    int random = 0;
    char strtmp[40] = "";
    struct MD5Context md5;
    unsigned char digest[16];
    char StatectlSession[64];         //the unique session of one Statectl Room	
    int x;

	gettimeofday(&now, NULL);
	srand((int)time(0));
	random = 1000 + rand()%9000;
	sprintf(strtmp,"%s-%ld%ld-%d", cnfno, now.tv_sec, now.tv_usec, random);

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *)strtmp, strlen(strtmp));
	MD5Final(digest, &md5);
	for (x=0;x<16;x++){
		sprintf(out_uniqueid + (x << 1), "%2.2x", digest[x]); /* safe */
	}
	sprintf(StatectlSession, "%s %s", out_uniqueid, cnfno); 
	strcpy(out_uniqueid, StatectlSession); 
	ast_log(LOG_DEBUG, "generate_statectl_uniqueid: [%s]\n", out_uniqueid);
}


static int get_addr_from_Mixing(char *rm_session, int isnew, int retry, struct sockaddr_in *out_addinfo){
	int sockfd = -1;
	struct sockaddr_in rsaddr;
	char sendbuf[512],  recvbuf[512] ;
	struct mixing_resp *resp = NULL;
	int recv_num = 0, len;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		ast_log(LOG_ERROR, "Unable to create network socket to Mixing server: %s\n", strerror(errno));
		return -1;
	}

	memset(&rsaddr, 0, sizeof(rsaddr));

	if (out_addinfo->sin_port>0)
	{
		memcpy(&rsaddr, out_addinfo, sizeof(struct sockaddr_in));
		if (isnew)
			ast_log(LOG_DEBUG, "--------groupsctl is Work----------\n");
	}
	else if (Mixing_media_port>0)
	{
		rsaddr.sin_family = AF_INET;
		rsaddr.sin_port = htons(Mixing_media_port);
		rsaddr.sin_addr.s_addr = inet_addr(Mixing_media_ip);
		ast_log(LOG_DEBUG, "--------statectl.conf is Work----------\n");
	}
	else
	{
		ast_log(LOG_WARNING, "--------Please check if the MixingServer address [%s:%d] is correct?----------\n",ast_inet_ntoa(rsaddr.sin_addr), ntohs(rsaddr.sin_port));
		return -1;
	}

	memset(sendbuf, 0x00, sizeof(sendbuf));
	if(!isnew){
		sprintf(sendbuf, "HUPCNF %s",  rm_session);
		ast_log(LOG_DEBUG, "send request : [%s] to [%s:%d]\n", sendbuf, ast_inet_ntoa(out_addinfo->sin_addr), ntohs(out_addinfo->sin_port));
		sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)out_addinfo, sizeof(struct sockaddr_in));
		sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)out_addinfo, sizeof(struct sockaddr_in));
		return 0;
	}
	else
	{
		ast_log(LOG_DEBUG, "----MixingServer address [%s:%d]--\n",ast_inet_ntoa(rsaddr.sin_addr), ntohs(rsaddr.sin_port));
		sprintf(sendbuf, "NEWCNF %s",  rm_session);
		ast_log(LOG_DEBUG, "send request : [%s] to [%s:%d]\n", sendbuf, ast_inet_ntoa(rsaddr.sin_addr), ntohs(rsaddr.sin_port));
		sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&rsaddr, sizeof(rsaddr));
	}

	while(retry){
		memset(out_addinfo, 0x00, sizeof(struct sockaddr_in));		
		retry--;	
		
		//set timeout
		struct timeval tv_out;
		tv_out.tv_sec = 0;
		tv_out.tv_usec = 100000;
		setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));

		memset(recvbuf, 0x00, sizeof(recvbuf));
		recv_num = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&rsaddr, (socklen_t *)&len);
		if(recv_num < 0){
			if(retry <= 0){
				ast_log(LOG_DEBUG, "Mixing receive error : %s\n", strerror(errno));
				close(sockfd);
				return -1;
			}else{
				continue;
			}
		}else{
			resp = (struct mixing_resp *)recvbuf;
			if ((strncmp(recvbuf, sendbuf, 6)==0)&&(strncmp(resp->id, &(sendbuf[7]), strlen(resp->id))==0)){
				resp->addr.sin_addr.s_addr = rsaddr.sin_addr.s_addr;
				memcpy(out_addinfo, &resp->addr, sizeof(struct sockaddr_in));
				ast_log(LOG_DEBUG, "receive response : %s : %s : %s:%d\n", resp->action, resp->id, ast_inet_ntoa(resp->addr.sin_addr), ntohs(resp->addr.sin_port));
				close(sockfd);
				return 0;
			}else{
				ast_log(LOG_DEBUG, "receive other response : %s : %s : %s:%d\n", resp->action, resp->id, ast_inet_ntoa(resp->addr.sin_addr), ntohs(resp->addr.sin_port));
				if(retry > 0)
					continue;
			}
		}
	}
	close(sockfd);
	return -1;
}


static char *istalking(int x)
{
	if (x > 0)
		return "(talking)";
	else if (x < 0)
		return "(unmonitored)";
	else 
		return "(not talking)";
}

#define	ZT_IOMUX_WRITE	2
#define	ZT_IOMUX_SIGEVENT	8
#define ZT_CODE	'J'
#define ZT_IOMUX		_IOWR (ZT_CODE, 9, int)


static int careful_write(int fd, unsigned char *data, int len, int block)
{
	int res;
	int x;

	while (len) {
		if (block) {
			x = ZT_IOMUX_WRITE | ZT_IOMUX_SIGEVENT;
			res = ioctl(fd, ZT_IOMUX, &x);
		} else
			res = 0;
		if (res >= 0)
			res = write(fd, data, len);
		if (res < 1) {
			if (errno != EAGAIN) {
				ast_log(LOG_DEBUG, "Failed to write audio data to conference: %s\n", strerror(errno));
				return -1;
			} else
				return 0;
		}
		len -= res;
		data += res;
	}

	return 0;
}

static int set_talk_volume(struct ast_conf_user *user, int volume)
{
	char gain_adjust;

	/* attempt to make the adjustment in the channel driver;
	   if successful, don't adjust in the frame reading routine
	*/
	gain_adjust = gain_map[volume + 5];

	return ast_channel_setoption(user->chan, AST_OPTION_RXGAIN, &gain_adjust, sizeof(gain_adjust), 0);
}

static int set_listen_volume(struct ast_conf_user *user, int volume)
{
	char gain_adjust;

	/* attempt to make the adjustment in the channel driver;
	   if successful, don't adjust in the frame reading routine
	*/
	gain_adjust = gain_map[volume + 5];

	return ast_channel_setoption(user->chan, AST_OPTION_TXGAIN, &gain_adjust, sizeof(gain_adjust), 0);
}

static void tweak_volume(struct volume *vol, enum volume_action action)
{
	switch (action) {
	case VOL_UP:
		switch (vol->desired) { 
		case 5:
			break;
		case 0:
			vol->desired = 2;
			break;
		case -2:
			vol->desired = 0;
			break;
		default:
			vol->desired++;
			break;
		}
		break;
	case VOL_DOWN:
		switch (vol->desired) {
		case -5:
			break;
		case 2:
			vol->desired = 0;
			break;
		case 0:
			vol->desired = -2;
			break;
		default:
			vol->desired--;
			break;
		}
	}
}

static void tweak_talk_volume(struct ast_conf_user *user, enum volume_action action)
{
	tweak_volume(&user->talk, action);
	/* attempt to make the adjustment in the channel driver;
	   if successful, don't adjust in the frame reading routine
	*/
	if (!set_talk_volume(user, user->talk.desired))
		user->talk.actual = 0;
	else
		user->talk.actual = user->talk.desired;
}

static void tweak_listen_volume(struct ast_conf_user *user, enum volume_action action)
{
	tweak_volume(&user->listen, action);
	/* attempt to make the adjustment in the channel driver;
	   if successful, don't adjust in the frame reading routine
	*/
	if (!set_listen_volume(user, user->listen.desired))
		user->listen.actual = 0;
	else
		user->listen.actual = user->listen.desired;
}

static void reset_volumes(struct ast_conf_user *user)
{
	signed char zero_volume = 0;

	ast_channel_setoption(user->chan, AST_OPTION_TXGAIN, &zero_volume, sizeof(zero_volume), 0);
	ast_channel_setoption(user->chan, AST_OPTION_RXGAIN, &zero_volume, sizeof(zero_volume), 0);
}

static void conf_play(struct ast_channel *chan, struct ast_conference *conf, enum entrance_sound sound)
{
	unsigned char *data;
	int len;
	int res = -1;
	
	if (!ast_channel_softhangup_internal_flag(chan))
		res = ast_autoservice_start(chan);

	AST_LIST_LOCK(&confs);

	switch(sound) {
	case ENTER:
		data = enter;
		len = sizeof(enter);
		break;
	case LEAVE:
		data = leave;
		len = sizeof(leave);
		break;
	default:
		data = NULL;
		len = 0;
	}
	if (data) {
		careful_write(conf->fd, data, len, 1);
	}

	AST_LIST_UNLOCK(&confs);

	if (!res) 
		ast_autoservice_stop(chan);
}

//anita,group_num like pttno,mixerno and so on
static int realtime_groups_control(struct ast_conference *cnf){
	struct ast_variable *var;

	//select * from groups_control where group_no = '10000';
	var = ast_load_realtime("groupsctl", "group_no", cnf->confno, NULL);

	if(!var)
		return -1;

	while (var) {
		if (!strcasecmp(var->name, "auth_type"))
			cnf->auth_mode = atoi(var->value);
		else if(!strcasecmp(var->name, "business_type"))
			cnf->business_mode = atoi(var->value);
		else if(!strcasecmp(var->name, "ptt_addr"))
			inet_aton(var->value, &(cnf->pttip.sin_addr));
		else if(!strcasecmp(var->name, "ptt_port"))
			cnf->pttip.sin_port = htons(atoi(var->value));
		else if(!strcasecmp(var->name, "mixer_addr"))
			inet_aton(var->value, &(cnf->MixingServerIP.sin_addr));
		else if(!strcasecmp(var->name, "mixer_port"))
			cnf->MixingServerIP.sin_port = htons(atoi(var->value));

		var = var->next;
	}
	ast_variables_destroy(var);

	return 0;
}
static int realtime_groups_whitelist(char *freeppnum, char *groupnum){
	struct ast_variable *var;
	int num = 0;

	//select * from groups_whitelist where group_no = '10000';
	var = ast_load_realtime("groupslist", "group_no", groupnum, "freepp", freeppnum, NULL);

	if(!var)
		return -1;

	while (var) {
		if (!strcasecmp(var->name, "freepp"))
			num++;
		var = var->next;
	}

	return num;
}

/*!
 * \brief Find or create a conference
 *
 * \param confno The conference name/number
 * \param pin The regular user pin
 * \param pinadmin The admin pin
 * \param make Make the conf if it doesn't exist
 * \param dynamic Mark the newly created conference as dynamic
 * \param refcount How many references to mark on the conference
 *
 * \return A pointer to the conference struct, or NULL if it wasn't found and
 *         make or dynamic were not set.
 */

static struct ast_conference *build_conf(char *confno, char *pin, char *pinadmin, char *maxcount, int make, int dynamic, int refcount)//anita-statectl
{
	struct ast_conference *cnf;
	int confno_int = 0;
	int retval=-1;

	AST_LIST_LOCK(&confs);

	AST_LIST_TRAVERSE(&confs, cnf, list) {
		if (!strcmp(confno, cnf->confno)) 
			break;
	}

	if (cnf || (!make && !dynamic))
		goto cnfout;

	/* Make a new one */
	if (!(cnf = ast_calloc(1, sizeof(*cnf))))
		goto cnfout;

	ast_mutex_init(&cnf->playlock);
	ast_mutex_init(&cnf->listenlock);
	cnf->recordthread = AST_PTHREADT_NULL;
	ast_mutex_init(&cnf->recordthreadlock);
	
	cnf->soundthread = AST_PTHREADT_NULL;
	ast_mutex_init(&cnf->soundthreadlock); //wind add sound lock in 20090122
	
	ast_copy_string(cnf->confno, confno, sizeof(cnf->confno));
	ast_copy_string(cnf->pin, pin, sizeof(cnf->pin));
	ast_copy_string(cnf->pinadmin, pinadmin, sizeof(cnf->pinadmin));
	ast_copy_string(cnf->maxcount, maxcount, sizeof(cnf->maxcount));//anita-statectl

	cnf->fd = -1;
	cnf->zapconf = -1;

	realtime_groups_control(cnf);
	ast_log(LOG_DEBUG,"------111111111groups %s : authmod=%d busimod=%d---\n",cnf->confno,cnf->auth_mode,cnf->business_mode);
	ast_log(LOG_DEBUG,"------222222222groups %s : pttserver address is %s:%d---\n",cnf->confno,ast_inet_ntoa(cnf->pttip.sin_addr), ntohs(cnf->pttip.sin_port));
	ast_log(LOG_DEBUG,"------333333333groups %s : mixer address is %s:%d---\n",cnf->confno,ast_inet_ntoa(cnf->MixingServerIP.sin_addr), ntohs(cnf->MixingServerIP.sin_port));

	if(cnf->auth_mode<=0 || cnf->auth_mode > 4 || cnf->business_mode<=0 || cnf->business_mode>3){
		ast_log(LOG_ERROR,"------conf %s set error auth_mode and business_mode so close this conference---\n",cnf->confno);
		free(cnf);
		cnf = NULL;
		goto cnfout;
	}

	/* Setup a new channel for playback of audio files */
	//get MixingServerIP
	if((cnf->business_mode == BUSI_AUDIO_CNF) || (cnf->business_mode == BUSI_VEDIO_CNF))
	{
		generate_statectl_uniqueid(cnf->confno, cnf->StatectlSession);
		retval=get_addr_from_Mixing(cnf->StatectlSession,  1,  2, &(cnf->MixingServerIP));
		if (retval != 0)
		{
			ast_log(LOG_WARNING, "Unable to get MixingServer Room's Address\n");
			if (cnf->fd >= 0)
				close(cnf->fd);
			free(cnf);
			cnf = NULL;
			goto cnfout;
		}		
	}

	struct ptt_request req;
	struct ptt_response rep;
	memset(&req, 0, sizeof(struct ptt_request));
	memset(&rep, 0, sizeof(struct ptt_response));
	req.BandMode = PTT_FOREVER_BAND;
	memcpy(req.BandCallNumber, cnf->confno, sizeof(req.BandCallNumber));
	if(cnf->business_mode == BUSI_PTT){
		memcpy(&cnf->pttip_tmp, &cnf->pttip, sizeof(struct sockaddr_in));
		ast_get_ptt_response(&req, 3, &rep, cnf->pttip_tmp);
		ast_log(LOG_DEBUG,"ptt get response : bandno %s : sessionid %s, pttserver %s:%d\n",
				cnf->confno, rep.id, ast_inet_ntoa(rep.addr.sin_addr), ntohs(rep.addr.sin_port));
		memcpy(&cnf->pttip, &rep.addr, sizeof(struct sockaddr_in));
		memcpy(cnf->pttid, rep.id, sizeof(cnf->pttid));
		if(!cnf->pttip.sin_addr.s_addr || !cnf->pttip.sin_port){
			ast_log(LOG_WARNING,"ptt get address error so remove the conference\n");
			free(cnf);
			cnf = NULL;
			goto cnfout;
		}
	}
	
	cnf->chan =NULL;

	/* Fill the conference struct */
	cnf->start = time(NULL);
	cnf->isdynamic = dynamic ? 1 : 0;
	if (option_verbose > 2)
		ast_verbose(VERBOSE_PREFIX_3 "Created StateCtl conference %d for conference '%s'\n", cnf->zapconf, cnf->confno);
	AST_LIST_INSERT_HEAD(&confs, cnf, list);

	/* Reserve conference number in map */
	if ((sscanf(cnf->confno, "%d", &confno_int) == 1) && (confno_int >= 0 && confno_int < 1024))
		conf_map[confno_int] = 1;
	
cnfout:
	if (cnf)
		ast_atomic_fetchadd_int(&cnf->refcount, refcount);

	AST_LIST_UNLOCK(&confs);

	return cnf;
}

static struct ast_conf_user *find_user(struct ast_conference *conf, char *callerident) 
{
	struct ast_conf_user *user = NULL;
	int cid;
	
	sscanf(callerident, "%i", &cid);
	if (conf && callerident) {
		AST_LIST_TRAVERSE(&conf->userlist, user, list) {
			if (cid == user->user_no)
				return user;
		}
	}
	return NULL;
}

static int dispose_conf(struct ast_conference *conf);
static int admin_exec(struct ast_channel *chan, const char *data){
	char *params;
	struct ast_conference *cnf;
	struct ast_conf_user *user = NULL;
	struct ast_module_user *u;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(confno);
		AST_APP_ARG(command);
		AST_APP_ARG(user);
	);

	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "StateCtlAdmin requires an argument!\n");
		return -1;
	}

	u = ast_module_user_add(chan);

	AST_LIST_LOCK(&confs);
	
	params = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, params);

	if (!args.command) {
		ast_log(LOG_WARNING, "StateCtlAdmin requires a command!\n");
		AST_LIST_UNLOCK(&confs);
		ast_module_user_remove(u);
		return -1;
	}
	AST_LIST_TRAVERSE(&confs, cnf, list) {
		if (!strcmp(cnf->confno, args.confno))
			break;
	}

	if (!cnf) {
		ast_log(LOG_WARNING, "Conference number '%s' not found!\n", args.confno);
		AST_LIST_UNLOCK(&confs);
		ast_module_user_remove(u);
		return 0;
	}

	ast_atomic_fetchadd_int(&cnf->refcount, 1);

	if (args.user)
		user = find_user(cnf, args.user);

	switch (*args.command) {
	case 76: /* L: Lock */ 
		cnf->locked = 1;
		break;
	case 108: /* l: Unlock */ 
		cnf->locked = 0;
		break;
	case 75: /* K: kick all users */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			user->adminflags |= ADMINFLAG_KICKME;
		break;
	case 101: /* e: Eject last user*/
		user = AST_LIST_LAST(&cnf->userlist);
		if (!ast_test_flag64(&user->userflags, CONFFLAG_ADMIN))
			user->adminflags |= ADMINFLAG_KICKME;
		else
			ast_log(LOG_NOTICE, "Not kicking last user, is an Admin!\n");
		break;
	case 77: /* M: Mute */ 
		if (user) {
			user->adminflags |= ADMINFLAG_MUTED;
		} else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 78: /* N: Mute all (non-admin) users */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list) {
			if (!ast_test_flag64(&user->userflags, CONFFLAG_ADMIN))
				user->adminflags |= ADMINFLAG_MUTED;
		}
		break;					
	case 109: /* m: Unmute */ 
		if (user) {
			user->adminflags &= ~(ADMINFLAG_MUTED | ADMINFLAG_SELFMUTED);
		} else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 110: /* n: Unmute all users */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			user->adminflags &= ~(ADMINFLAG_MUTED | ADMINFLAG_SELFMUTED);
		break;
	case 107: /* k: Kick user */ 
		if (user)
			user->adminflags |= ADMINFLAG_KICKME;
		else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 118: /* v: Lower all users listen volume */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			tweak_listen_volume(user, VOL_DOWN);
		break;
	case 86: /* V: Raise all users listen volume */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			tweak_listen_volume(user, VOL_UP);
		break;
	case 115: /* s: Lower all users speaking volume */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			tweak_talk_volume(user, VOL_DOWN);
		break;
	case 83: /* S: Raise all users speaking volume */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			tweak_talk_volume(user, VOL_UP);
		break;
	case 82: /* R: Reset all volume levels */
		AST_LIST_TRAVERSE(&cnf->userlist, user, list)
			reset_volumes(user);
		break;
	case 114: /* r: Reset user's volume level */
		if (user)
			reset_volumes(user);
		else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 85: /* U: Raise user's listen volume */
		if (user)
			tweak_listen_volume(user, VOL_UP);
		else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 117: /* u: Lower user's listen volume */
		if (user)
			tweak_listen_volume(user, VOL_DOWN);
		else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 84: /* T: Raise user's talk volume */
		if (user)
			tweak_talk_volume(user, VOL_UP);
		else
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	case 116: /* t: Lower user's talk volume */
		if (user) 
			tweak_talk_volume(user, VOL_DOWN);
		else 
			ast_log(LOG_NOTICE, "Specified User not found!\n");
		break;
	}

	AST_LIST_UNLOCK(&confs);

	dispose_conf(cnf);

	ast_module_user_remove(u);
	
	return 0;
}


static char *complete_confno(const char *word, int state)
{
	struct ast_conference *cnf;
	char *ret = NULL;
	int which = 0;
	int len = strlen(word);

	AST_LIST_LOCK(&confs);
	AST_LIST_TRAVERSE(&confs, cnf, list) {
		if (!strncmp(word, cnf->confno, len) && ++which > state) {
			/* dup before releasing the lock */
			ret = ast_strdup(cnf->confno);
			break;
		}
	}
	AST_LIST_UNLOCK(&confs);
	return ret;
}

static char *complete_userno(struct ast_conference *cnf, const char *word, int state)
{
	char usrno[50];
	struct ast_conf_user *usr;
	char *ret = NULL;
	int which = 0;
	int len = strlen(word);
	if (cnf) {
		/* Search for the user */
		AST_LIST_LOCK(&confs);
		AST_LIST_TRAVERSE(&cnf->userlist, usr, list) {
			snprintf(usrno, sizeof(usrno), "%d", usr->user_no);
			if (!strncasecmp(word, usrno, len) && ++which > state)
			{
				ret = ast_strdup(usrno);
				break;
			}
		}
		AST_LIST_UNLOCK(&confs);
	}
	return ret;
}

static char *complete_statectlcmd_mute_kick(const char *line, const char *word, int pos, int state)
{
	if (pos == 2) {
		return complete_confno(word, state);
	}
	if (pos == 3) {
		int len = strlen(word);
		char *ret = NULL;
		char *saved = NULL;
		char *myline;
		char *confno;
		struct ast_conference *cnf;

		if (!strncasecmp(word, "all", len)) {
			if (state == 0) {
				return ast_strdup("all");
			}
			--state;
		}

		/* Extract the confno from the command line. */
		myline = ast_strdupa(line);
		strtok_r(myline, " ", &saved);
		strtok_r(NULL, " ", &saved);
		confno = strtok_r(NULL, " ", &saved);

		AST_LIST_LOCK(&confs);
		AST_LIST_TRAVERSE(&confs, cnf, list) {
			if (!strcmp(confno, cnf->confno)) {
				ret = complete_userno(cnf, word, state);
				break;
			}
		}
		AST_LIST_UNLOCK(&confs);

		return ret;
	}
	return NULL;
}

static char *complete_statectlcmd_list(const char *line, const char *word, int pos, int state)
{
	int len;

	if (pos == 2) {
		len = strlen(word);
		if (!strncasecmp(word, STR_CONCISE, len)) {
			if (state == 0) {
				return ast_strdup(STR_CONCISE);
			}
			--state;
		}

		return complete_confno(word, state);
	}
	if (pos == 3 && state == 0) {
		char *saved = NULL;
		char *myline;
		char *confno;

		/* Extract the confno from the command line. */
		myline = ast_strdupa(line);
		strtok_r(myline, " ", &saved);
		strtok_r(NULL, " ", &saved);
		confno = strtok_r(NULL, " ", &saved);

		if (!strcasecmp(confno, STR_CONCISE)) {
			/* There is nothing valid in this position now. */
			return NULL;
		}

		len = strlen(word);
		if (!strncasecmp(word, STR_CONCISE, len)) {
			return ast_strdup(STR_CONCISE);
		}
	}
	return NULL;
}

static char *statectl_show_cmd(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
{
	/* Process the command */
	struct ast_conf_user *user;
	struct ast_conference *cnf;
	int hr, min, sec;
	int total = 0;
	time_t now;
#define MC_HEADER_FORMAT "%-14s %-14s %-10s %-8s  %-8s  %-6s\n"
#define MC_DATA_FORMAT "%-12.12s   %4.4d	      %4.4s       %02d:%02d:%02d  %-8s  %-6s\n"

	switch (cmd) {
	case CLI_INIT:
		e->command = "statectl list";
		e->usage =
			"Usage: statectl list [<confno>] [" STR_CONCISE "]\n"
			"       List all conferences or a specific conference.\n";
		return NULL;
	case CLI_GENERATE:
		return complete_statectlcmd_list(a->line, a->word, a->pos, a->n);
	}

	if (a->argc == 2 || (a->argc == 3 && !strcasecmp(a->argv[2], STR_CONCISE))) {
		/* List all the conferences */
		int concise = (a->argc == 3);
		struct ast_str *marked_users;

		if (!(marked_users = ast_str_create(30))) {
			return CLI_FAILURE;
		}

		now = time(NULL);
		AST_LIST_LOCK(&confs);
		if (AST_LIST_EMPTY(&confs)) {
			if (!concise) {
				ast_cli(a->fd, "No active StateCtl conferences.\n");
			}
			AST_LIST_UNLOCK(&confs);
			ast_free(marked_users);
			return CLI_SUCCESS;
		}
		if (!concise) {
			ast_cli(a->fd, MC_HEADER_FORMAT, "Conf Num", "Parties", "Marked", "Activity", "Creation", "Locked");
		}
		AST_LIST_TRAVERSE(&confs, cnf, list) {
			hr = (now - cnf->start) / 3600;
			min = ((now - cnf->start) % 3600) / 60;
			sec = (now - cnf->start) % 60;
			if (!concise) {
				if (cnf->markedusers == 0) {
					ast_str_set(&marked_users, 0, "N/A ");
				} else {
					ast_str_set(&marked_users, 0, "%4.4d", cnf->markedusers);
				}
				ast_cli(a->fd, MC_DATA_FORMAT, cnf->confno, cnf->users,
					ast_str_buffer(marked_users), hr, min, sec,
					cnf->isdynamic ? "Dynamic" : "Static", cnf->locked ? "Yes" : "No");
			} else {
				ast_cli(a->fd, "%s!%d!%d!%02d:%02d:%02d!%d!%d\n",
					cnf->confno,
					cnf->users,
					cnf->markedusers,
					hr, min, sec,
					cnf->isdynamic,
					cnf->locked);
			}

			total += cnf->users;
		}
		AST_LIST_UNLOCK(&confs);
		if (!concise) {
			ast_cli(a->fd, "* Total number of StateCtl users: %d\n", total);
		}
		ast_free(marked_users);
		return CLI_SUCCESS;
	}
	if (a->argc == 3 || (a->argc == 4 && !strcasecmp(a->argv[3], STR_CONCISE))) {
		int concise = (a->argc == 4);

		/* List all the users in a conference */
		if (AST_LIST_EMPTY(&confs)) {
			if (!concise) {
				ast_cli(a->fd, "No active StateCtl conferences.\n");
			}
			return CLI_SUCCESS;
		}
		/* Find the right conference */
		AST_LIST_LOCK(&confs);
		AST_LIST_TRAVERSE(&confs, cnf, list) {
			if (strcmp(cnf->confno, a->argv[2]) == 0) {
				break;
			}
		}
		if (!cnf) {
			if (!concise)
				ast_cli(a->fd, "No such conference: %s.\n", a->argv[2]);
			AST_LIST_UNLOCK(&confs);
			return CLI_SUCCESS;
		}
		/* Show all the users */
		time(&now);
		AST_LIST_TRAVERSE(&cnf->userlist, user, list) {
			hr = (now - user->jointime) / 3600;
			min = ((now - user->jointime) % 3600) / 60;
			sec = (now - user->jointime) % 60;
			if (user->user_status==1)
			{	
			if ( !concise )
				ast_cli(a->fd, "User #: %-2.2d %12.12s %-20.20s %-1.1d Channel: %s %s %s %s %s %02d:%02d:%02d\n",
					user->user_no,
					S_COR(ast_channel_caller(user->chan)->id.number.valid, ast_channel_caller(user->chan)->id.number.str, "<unknown>"),
					S_COR(ast_channel_caller(user->chan)->id.name.valid, ast_channel_caller(user->chan)->id.name.str, "<unknown>"),
					user->user_status,
					//user->chan->name,
					ast_channel_name(user->chan),
					ast_test_flag64(&user->userflags,  CONFFLAG_ADMIN) ? "(Admin)" : "",
					ast_test_flag64(&user->userflags, CONFFLAG_MONITOR) ? "(Listen only)" : "",
					user->adminflags & ADMINFLAG_MUTED ? "(Admin Muted)" : user->adminflags & ADMINFLAG_SELFMUTED ? "(Muted)" : "",
					istalking(user->talking), hr, min, sec); 
			else 
				ast_cli(a->fd, "%d!%s!%s!%s!%s!%s!%s!%d!%02d:%02d:%02d\n",
					user->user_no,
					S_COR(ast_channel_caller(user->chan)->id.number.valid, ast_channel_caller(user->chan)->id.number.str, ""),
					S_COR(ast_channel_caller(user->chan)->id.name.valid, ast_channel_caller(user->chan)->id.name.str, ""),
					//user->chan->name,
					ast_channel_name(user->chan),
					ast_test_flag64(&user->userflags, CONFFLAG_ADMIN)   ? "1" : "",
					ast_test_flag64(&user->userflags, CONFFLAG_MONITOR) ? "1" : "",
					user->adminflags & (ADMINFLAG_MUTED | ADMINFLAG_SELFMUTED)  ? "1" : "",
					user->talking, hr, min, sec);
			}
			else
			{
				ast_cli(a->fd, "User #: %-2.2d %12.12s %-20.20s %-1.1d \n",
					user->user_no,
					S_OR(user->cid_num, "<unknown>"),
					S_OR(user->cid_name, "<no name>"),
					user->user_status); 		
			}
		}
		if (!concise) {
			ast_cli(a->fd,"%d users in that conference.count user %d\n",cnf->users, cnf->user_count);
		}
		AST_LIST_UNLOCK(&confs);
		return CLI_SUCCESS;
	}
	return CLI_SHOWUSAGE;
}


static char *statectl_cmd_helper(struct ast_cli_args *a)
{
	/* Process the command */
	struct ast_str *cmdline;

	/* Max confno length */
	if (!(cmdline = ast_str_create(MAX_CONFNUM))) {
		return CLI_FAILURE;
	}

	ast_str_set(&cmdline, 0, "%s", a->argv[2]);	/* Argv 2: conference number */
	if (strcasestr(a->argv[1], "lock")) {
		if (strcasecmp(a->argv[1], "lock") == 0) {
			/* Lock */
			ast_str_append(&cmdline, 0, ",L");
		} else {
			/* Unlock */
			ast_str_append(&cmdline, 0, ",l");
		}
	} else if (strcasestr(a->argv[1], "mute")) { 
		if (strcasecmp(a->argv[1], "mute") == 0) {
			/* Mute */
			if (strcasecmp(a->argv[3], "all") == 0) {
				ast_str_append(&cmdline, 0, ",N");
			} else {
				ast_str_append(&cmdline, 0, ",M,%s", a->argv[3]);	
			}
		} else {
			/* Unmute */
			if (strcasecmp(a->argv[3], "all") == 0) {
				ast_str_append(&cmdline, 0, ",n");
			} else {
				ast_str_append(&cmdline, 0, ",m,%s", a->argv[3]);
			}
		}
	} else if (strcasecmp(a->argv[1], "kick") == 0) {
		if (strcasecmp(a->argv[3], "all") == 0) {
			/* Kick all */
			ast_str_append(&cmdline, 0, ",K");
		} else {
			/* Kick a single user */
			ast_str_append(&cmdline, 0, ",k,%s", a->argv[3]);
		}
	} else {
		/*
		 * Should never get here because it is already filtered by the
		 * callers.
		 */
		ast_free(cmdline);
		return CLI_SHOWUSAGE;
	}

	ast_debug(1, "Cmdline: %s\n", ast_str_buffer(cmdline));

	admin_exec(NULL, ast_str_buffer(cmdline));
	ast_free(cmdline);

	return CLI_SUCCESS;
}

static char *statectl_kick_cmd(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
{
	switch (cmd) {
	case CLI_INIT:
		e->command = "statectl kick";
		e->usage =
			"Usage: statectl kick <confno> all|<userno>\n"
			"       Kick a conference or a user in a conference.\n";
		return NULL;
	case CLI_GENERATE:
		return complete_statectlcmd_mute_kick(a->line, a->word, a->pos, a->n);
	}

	if (a->argc != 4) {
		return CLI_SHOWUSAGE;
	}

	return statectl_cmd_helper(a);
}

static char *statectl_mute_cmd(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
{
	switch (cmd) {
	case CLI_INIT:
		e->command = "statectl {mute|unmute}";
		e->usage =
			"Usage: statectl mute|unmute <confno> all|<userno>\n"
			"       Mute or unmute a conference or a user in a conference.\n";
		return NULL;
	case CLI_GENERATE:
		return complete_statectlcmd_mute_kick(a->line, a->word, a->pos, a->n);
	}

	if (a->argc != 4) {
		return CLI_SHOWUSAGE;
	}

	return statectl_cmd_helper(a);
}


static struct ast_cli_entry cli_statectl[] = {
	AST_CLI_DEFINE(statectl_kick_cmd, "Kick a conference or a user in a conference."),
	AST_CLI_DEFINE(statectl_show_cmd, "List all conferences or a specific conference."),
	AST_CLI_DEFINE(statectl_mute_cmd, "Mute or unmute a conference or a user in a conference."),
};

#define	ZT_FLUSH_READ		1

/* Flush and stop the write (output) process */
#define	ZT_FLUSH_WRITE		2

/* Flush the event queue */
#define	ZT_FLUSH_EVENT		4

/* Flush everything */
#define	ZT_FLUSH_ALL		(ZT_FLUSH_READ | ZT_FLUSH_WRITE | ZT_FLUSH_EVENT)
#define	ZT_FLUSH		_IOW (ZT_CODE, 3, int)

static void conf_flush(int fd, struct ast_channel *chan)
{
	int x;

	/* read any frames that may be waiting on the channel
	   and throw them away
	*/
	if (chan) {
		struct ast_frame *f;

		/* when no frames are available, this will wait
		   for 1 millisecond maximum
		*/
		while (ast_waitfor(chan, 1)) {
			f = ast_read(chan);
			ast_log(LOG_DEBUG, "+++++++++++++++++++++++\n");
			if (f)
				ast_frfree(f);
			else /* channel was hung up or something else happened */
				break;
		}
	}

	/* flush any data sitting in the pseudo channel */
	x = ZT_FLUSH_ALL;
	if (ioctl(fd, ZT_FLUSH, &x))
		ast_log(LOG_DEBUG, "Error flushing channel\n");

}

/* Remove the conference from the list and free it.
   We assume that this was called while holding conflock. */
static int conf_free(struct ast_conference *conf)
{
	int x;
	
	AST_LIST_REMOVE(&confs, conf, list);

	if (conf->recording == STATECTL_RECORD_ACTIVE) {
		conf->recording = STATECTL_RECORD_TERMINATE;
		AST_LIST_UNLOCK(&confs);
		while (1) {
			usleep(1);
			AST_LIST_LOCK(&confs);
			if (conf->recording == STATECTL_RECORD_OFF)
				break;
			AST_LIST_UNLOCK(&confs);
		}
	}

	for (x=0;x<AST_FRAME_BITS;x++) {
		if (conf->transframe[x])
			ast_frfree(conf->transframe[x]);
		if (conf->transpath[x])
			ast_translator_free_path(conf->transpath[x]);
	}
	if (conf->origframe)
		ast_frfree(conf->origframe);
	if (conf->lchan)
		ast_hangup(conf->lchan);
	if (conf->chan)
	{
		ast_log(LOG_DEBUG, "=====close statectl zap channel====\n");
		ast_hangup(conf->chan);
	}
	if (conf->fd >= 0)
		close(conf->fd);
	//send MixingServer Stop
	if((conf->business_mode == BUSI_AUDIO_CNF) || (conf->business_mode == BUSI_VEDIO_CNF))//anita	
		get_addr_from_Mixing(conf->StatectlSession,  0,  1, &(conf->MixingServerIP));
	if(conf->business_mode == BUSI_PTT) //anita	
	{
		struct ptt_request req;
		struct ptt_response rep;
		memset(&req, 0, sizeof(struct ptt_request));
		memset(&rep, 0, sizeof(struct ptt_response));
		req.BandMode = PTT_FOREVER_BAND;
		sprintf(req.action, "%s", "HUPPT");
		memcpy(req.BandCallNumber, conf->confno, sizeof(req.BandCallNumber));
		ast_get_ptt_response(&req, 1, &rep, conf->pttip_tmp);		
	}

	ast_mutex_destroy(&conf->playlock);
	ast_mutex_destroy(&conf->listenlock);
	ast_mutex_destroy(&conf->soundthreadlock);
	ast_mutex_destroy(&conf->recordthreadlock);
	free(conf);

	return 0;
}

/* Decrement reference counts, as incremented by find_conf() */
static int dispose_conf(struct ast_conference *conf)
{
	int res = 0;
	int confno_int = 0;
	struct ast_conf_user *t_usr;

	AST_LIST_LOCK(&confs);
	if (ast_atomic_dec_and_test(&conf->refcount)) {
		/* Take the conference room number out of an inuse state */
		if ((sscanf(conf->confno, "%d", &confno_int) == 1) && (confno_int >= 0 && confno_int < 1024))
			conf_map[confno_int] = 0;
		ast_log(LOG_DEBUG, "=====close statectl zap channel====\n");
		AST_LIST_TRAVERSE_SAFE_BEGIN(&conf->userlist, t_usr, list) {
			if(!t_usr)								
				break;
			ast_log(LOG_DEBUG, "================t_usr->cid_num=%s=========\n", t_usr->cid_num);
			AST_LIST_REMOVE_CURRENT(list);
			free(t_usr);
		}
		AST_LIST_TRAVERSE_SAFE_END
		conf_free(conf);
		res = 1;
	}
	AST_LIST_UNLOCK(&confs);

	return res;
}

static int conf_run(struct ast_channel *chan, struct ast_conference *conf, struct ast_flags64 *confflags, char *optargs[])
{
	struct ast_conf_user *user = NULL;
	int i_hangup=0;
	int fd;
	struct ast_frame *f;
	struct ast_channel *c;
	int outfd;
	int ms;
	int nfds;
	int res;
	int origfd;
	int firstpass = 0;
	int ret = -1;
	int hr, min, sec;
	int sent_event = 0;
	time_t now;
	char statectlsecs[30] = "";
	char members[10] = "";
	time_t timeout = 0;
	char __buf[CONF_SIZE + AST_FRIENDLY_OFFSET];
	char *buf = __buf + AST_FRIENDLY_OFFSET;
	int setusercount = 0;

	if (ast_channel_state(chan) != AST_STATE_UP)
		ast_answer(chan);

	int maxusers = atoi(conf->maxcount);
	if(maxusers && (conf->users >= maxusers)){
		ast_log(LOG_DEBUG,"-----conference %s has reached maxcount %d----\n",conf->confno, maxusers);
		return ret;
	}


	if (!(user = ast_calloc(1, sizeof(*user))))
		return ret;

	time(&user->jointime);

   	ast_mutex_lock(&conf->playlock);

	if (AST_LIST_EMPTY(&conf->userlist))
		user->user_no = 1;
	else
	{	
		struct ast_conf_user *t_usr;
		AST_LIST_TRAVERSE(&conf->userlist, t_usr, list) {
			if(!t_usr)								
				break;
			ast_log(LOG_DEBUG, "cid_num=%s===user_status=%d=mute_status=%d=hold_status=%d=\n", t_usr->cid_num, t_usr->user_status, t_usr->mute_status, t_usr->hold_status);
			ast_log(LOG_DEBUG, "id.number.str=%s==\n", ast_channel_caller(chan)->id.number.str);
			if ((strcmp(t_usr->cid_num, S_COR(ast_channel_caller(chan)->id.number.valid, ast_channel_caller(chan)->id.number.str, ""))==0) &&(t_usr->user_status==0))
			{
				AST_LIST_REMOVE(&conf->userlist, t_usr, list);
				conf->user_count--;
			}
		}
		user->user_no = AST_LIST_LAST(&conf->userlist)->user_no + 1;
	}
	AST_LIST_INSERT_TAIL(&conf->userlist, user, list);
	
	ast_copy_string(user->cid_num, S_COR(ast_channel_caller(chan)->id.number.valid, ast_channel_caller(chan)->id.number.str, "<unknown>"), sizeof(user->cid_num));
	ast_copy_string(user->cid_name, S_COR(ast_channel_caller(chan)->id.name.valid, ast_channel_caller(chan)->id.name.str, "<unknown>"), sizeof(user->cid_name));
	user->user_status=1;
	user->mute_status=0;
	user->hold_status=0;
	
	user->chan = chan;
	user->userflags = *confflags;
	user->adminflags = ast_test_flag64(confflags, CONFFLAG_STARTMUTED) ? ADMINFLAG_SELFMUTED : 0;
	user->talking = -1;
	
	conf->users++;
	conf->user_count++;
	/* Update table */
	snprintf(members, sizeof(members), "%d", conf->users);
	ast_update_realtime("statectl", "confno", conf->confno, "members", members , NULL);
	setusercount = 1;

	/* This device changed state now - if this is the first user */
	if (conf->users == 1)
		ast_devstate_changed(AST_DEVICE_INUSE, (conf->isdynamic ? AST_DEVSTATE_NOT_CACHABLE : AST_DEVSTATE_CACHABLE), "statectl:%s", conf->confno);

	ast_mutex_unlock(&conf->playlock);

	if ( !ast_test_flag64(confflags , (CONFFLAG_QUIET | CONFFLAG_NOONLYPERSON)) ) {
		
		if (conf->users == 1 && !ast_test_flag64(confflags, CONFFLAG_WAITMARKED))
		{
			if (!ast_streamfile(chan, "conf-onlyperson", ast_channel_language(chan)))
				ast_waitstream(chan, "");
		}
		if (ast_test_flag64(confflags, CONFFLAG_WAITMARKED) && conf->markedusers == 0)
			if (!ast_streamfile(chan, "conf-waitforleader", ast_channel_language(chan)))
				ast_waitstream(chan, "");
	}

	if (!ast_test_flag64(confflags , CONFFLAG_QUIET) && ast_test_flag64(confflags, CONFFLAG_ANNOUNCEUSERCOUNT) && conf->users > 1) {
		int keepplaying = 1;

		if (conf->users == 2) { 
			if (!ast_streamfile(chan,"conf-onlyone",ast_channel_language(chan))) {
				res = ast_waitstream(chan, AST_DIGIT_ANY);
				ast_stopstream(chan);
				if (res > 0)
					keepplaying=0;
				else if (res == -1)
					goto outrun;
			}
		} else { 
			if (!ast_streamfile(chan, "conf-thereare", ast_channel_language(chan))) {
				res = ast_waitstream(chan, AST_DIGIT_ANY);
				ast_stopstream(chan);
				if (res > 0)
					keepplaying=0;
				else if (res == -1)
					goto outrun;
			}
			if (keepplaying) {
				res = ast_say_number(chan, conf->users - 1, AST_DIGIT_ANY, ast_channel_language(chan), (char *) NULL);
				if (res > 0)
					keepplaying=0;
				else if (res == -1)
					goto outrun;
			}
			if (keepplaying && !ast_streamfile(chan, "conf-otherinparty", ast_channel_language(chan))) {
				res = ast_waitstream(chan, AST_DIGIT_ANY);
				ast_stopstream(chan);
				if (res > 0)
					keepplaying=0;
				else if (res == -1) 
					goto outrun;
			}
		}
	}

	ast_indicate(chan, -1);

	origfd = ast_channel_fd(chan, 0);
	/* XXX Make sure we're not running on a pseudo channel XXX */
	fd = ast_channel_fd(chan, 0);
	nfds = 0;
	
	ast_mutex_lock(&conf->playlock);

	if (!sent_event) {
		manager_event(EVENT_FLAG_CALL, "StatectlJoin", 
			      "Channel: %s\r\n"
			      "Uniqueid: %s\r\n"
			      "Statectl: %s\r\n"
			      "Usernum: %d\r\n",
			      ast_channel_name(chan), ast_channel_uniqueid(chan),  conf->confno, user->user_no);
		sent_event = 1;
	}

	if (!firstpass && !ast_test_flag64(confflags, CONFFLAG_MONITOR) && !ast_test_flag64(confflags, CONFFLAG_ADMIN)) {
		firstpass = 1;
		if (!ast_test_flag64(confflags, CONFFLAG_QUIET))
			if (!ast_test_flag64(confflags , CONFFLAG_WAITMARKED) || (ast_test_flag64(confflags, CONFFLAG_MARKEDUSER) && (conf->markedusers >= 1)))
				conf_play(chan, conf, ENTER);
	}

	/***************statectl status send to ALL************************/	
	struct ast_conf_user *t_usr1;
	struct ast_frame t_f1 = { AST_FRAME_TEXT, .subclass.integer=0 };
	char text2send1[256] = "";
	memset(text2send1, 0x00, sizeof(text2send1));
	sprintf(text2send1, "ICS {\"no\":%d,\"fi\":\"%s\",\"na\":\"%s\",\"st\":%d,\"mt\":%d,\"hd\":%d}",  0, S_COR(ast_channel_caller(chan)->id.number.valid, ast_channel_caller(chan)->id.number.str, "<unknown>"), 
				S_COR(ast_channel_caller(chan)->id.name.valid, ast_channel_caller(chan)->id.name.str, "<unknown>"), 1, 0, 0);
	t_f1.data.ptr = (void *)text2send1;
	t_f1.datalen = strlen(text2send1) + 1;
	
	AST_LIST_TRAVERSE(&conf->userlist, t_usr1, list) {							
		if(!t_usr1)								
			break;
		if ((t_usr1->user_status==1)&&(t_usr1->chan))
		{
			ast_log(LOG_DEBUG, "---status data=[%s]=sendto=t_usr1=[%s]---\n", text2send1, t_usr1->cid_num);
			ast_write(t_usr1->chan, &t_f1) ;
		}
	}
	/***************statectl status send to ALL************************/
	ast_mutex_unlock(&conf->playlock);

	conf_flush(fd, chan);

	if (ast_test_flag64(confflags, CONFFLAG_AGI)) {
		/* Get name of AGI file to run from $(STATECTL_AGI_BACKGROUND)
		   or use default filename of conf-background.agi */
		;
	} 
	else 
	{
		for(;;) {

			outfd = -1;
			ms = 100;
			
			if (timeout && time(NULL) >= timeout)
			{
				ast_log(LOG_DEBUG, "======timeout========\n");
				break;
			}

			c = ast_waitfor_nandfds(&chan, 1, &fd, nfds, NULL, &outfd, &ms);
			
			
			/* Update the struct with the actual confflags */
			user->userflags = *confflags;
			
			/* If I have been kicked, exit the conference */
			if (user->adminflags & ADMINFLAG_KICKME) {
				//You have been kicked.
				if (!ast_test_flag64(confflags, CONFFLAG_QUIET) && 
					!ast_streamfile(chan, "conf-kicked", ast_channel_language(chan))) {
					ast_waitstream(chan, "");
				}
				ret = 0;
				ast_log(LOG_DEBUG, "==========ADMINFLAG_KICKME========\n");
				break;
			}

			/* Perform an extra hangup check just in case */
			if (ast_check_hangup(chan))
			{
				ast_log(LOG_DEBUG, "==========ast_check_hangup=========\n");
				break;
			}

			if (c) {
				f = ast_read(c);
				if (!f)
				{
					ast_log(LOG_DEBUG, "==========ast_read fail=============\n");
					break;
				}
					
				//ast_log(LOG_DEBUG, "===================================================\n");
				if ((f->frametype == AST_FRAME_TEXT)&&(strncmp(f->data.ptr, "QCS", 3)==0))
				{
					ast_log(LOG_DEBUG, "----TEXT--request data=[%s]----\n", (char *)(f->data.ptr));	
					AST_LIST_LOCK(&confs);
					struct ast_conf_user *t_usr;
					struct ast_frame t_f = { AST_FRAME_TEXT, .subclass.integer=0 };
					char text2send[256] = "", t_str[128] ="";	
					int num=0, ti=0;
					memset(t_str, 0x00, sizeof(t_str));
					memset(text2send, 0x00, sizeof(text2send));
					memcpy(t_str, f->data.ptr, f->datalen);
					//sprintf(t_str, "QCS {\"ci\":\"%s\"}", "700001");
					char *tsubstr = "ci";
					char *ts = strstr(t_str, tsubstr);
					if (ts!=NULL)
					{
						ts = ts +5;
						for(ti=0; ti<strlen(ts); ti++)
    						{
        						if (*(ts+ti)==34)
       						{
            							*(ts+ti)='\0';
            							break;
        						}
    						}
						ast_log(LOG_DEBUG, "-------request meetme name--=[%s]----------\n", ts);									
						sprintf(text2send, "QCS {\"ci\":\"%s\",\"ct\":%d,\"pt\":%d}",  ts, conf->user_count, query_interval);
						t_f.data.ptr =  (void *)text2send;
						t_f.datalen = strlen(text2send) + 1;
						ast_log(LOG_DEBUG, "---response data=[%s]\n", text2send);	
						ast_write(chan, &t_f) ;					
						AST_LIST_TRAVERSE(&conf->userlist, t_usr, list) {							
							if(!t_usr)								
								break;
							memset(text2send, 0x00, sizeof(text2send));
							sprintf(text2send, "ICS {\"no\":%d,\"fi\":\"%s\",\"na\":\"%s\",\"st\":%d,\"mt\":%d,\"hd\":%d}",  ++num, S_OR(t_usr->cid_num, "<unknown>"), 
											S_OR(t_usr->cid_name, "<unknown>"), t_usr->user_status, t_usr->mute_status, t_usr->hold_status);
							t_f.data.ptr = ( void *)text2send;
							t_f.datalen = strlen(text2send) + 1;
							ast_log(LOG_DEBUG, "---response data=[%s]\n", text2send);
							ast_write(chan, &t_f) ;
						}
						AST_LIST_UNLOCK(&confs);
					}
				}
				//ast_log(LOG_DEBUG, "===================================================\n");		
				//ast_log(LOG_DEBUG, "===================================================\n");
				if ((f->frametype == AST_FRAME_TEXT)&&(strncmp(f->data.ptr, "KIC", 3)==0))
				{
					ast_log(LOG_DEBUG, "----TEXT--request data=[%s]----\n", (char *)(f->data.ptr));	
					AST_LIST_LOCK(&confs);
					struct ast_conf_user *k_usr;
					char k_str[128] ="";	
					int ki=0;
					memset(k_str, 0x00, sizeof(k_str));
					memcpy(k_str, f->data.ptr, f->datalen);
					//sprintf(k_str, "KIC {\"fi\":\"%s\"}", "24001234");
					char *ksubstr = "fi";
					char *ks = strstr(k_str, ksubstr);
					if (ks!=NULL)
					{
						ks = ks +5;
						for(ki=0; ki<strlen(ks); ki++)
    						{
        						if (*(ks+ki)==34)
       						{
            							*(ks+ki)='\0';
            							break;
        						}
    						}
						ast_log(LOG_DEBUG, "-------request user name--=[%s]----------\n", ks);	
						if ((strncmp(ks, "ALL", 3)==0)||(strncmp(ks, "all", 3)==0))
						{
							AST_LIST_TRAVERSE(&conf->userlist, k_usr, list) {							
								if(!k_usr)								
									break;
								k_usr->adminflags |= ADMINFLAG_KICKME;
							}
						}
						else
						{
							AST_LIST_TRAVERSE(&conf->userlist, k_usr, list) {							
								if(!k_usr)								
									break;
								if ((strcmp(k_usr->cid_num, ks)==0) &&(k_usr->user_status==1))
								{
									ast_log(LOG_DEBUG, "----kick-cid_num=[%s]--ks=[%s]---user_status=[%d]-----\n", k_usr->cid_num, ks, k_usr->user_status);	
									k_usr->adminflags |= ADMINFLAG_KICKME;
								}
							}
						}
						AST_LIST_UNLOCK(&confs);
					}
				}

				if ((f->frametype == AST_FRAME_TEXT)&&(strncmp(f->data.ptr, "MUT", 3)==0))
				{
					ast_log(LOG_DEBUG, "----TEXT--request data=[%s]----\n", (char *)(f->data.ptr));	
					AST_LIST_LOCK(&confs);
					struct ast_conf_user *m_usr;
					struct ast_frame m_f = { AST_FRAME_TEXT, .subclass.integer=0 };
					char m_text2send[256] = "", m_str[128] ="", m_str1[128] ="";	
					int  mi=0;
					memset(m_str, 0x00, sizeof(m_str));
					memset(m_text2send, 0x00, sizeof(m_text2send));
					memcpy(m_str, f->data.ptr, f->datalen);
					memcpy(m_str1, f->data.ptr, f->datalen);
					//sprintf(m_str, "MUT {\"fi\":\"%s\"}", "24001234");
					//MUT {"fi":"2400123","st":1}
					char *msubstr = "fi";
					char *msubstr1 = "st";
					char *mts = strstr(m_str, msubstr);
					char *mts1 = strstr(m_str1, msubstr1);
					int m_fag=0;
					if ((mts!=NULL)&&(mts1!=NULL))
					{
						mts = mts +5;
						for(mi=0; mi<strlen(mts); mi++)
    						{
        						if (*(mts+mi)==34)
       						{
            							*(mts+mi)='\0';
            							break;
        						}
    						}
						mts1 = mts1 +5;
						for(mi=0; mi<strlen(mts1); mi++)
    						{
        						if (*(mts1+mi)==34)
       						{
            							*(mts1+mi)='\0';
            							break;
        						}
    						}
						ast_log(LOG_DEBUG, "-------request user name--=[%s]--mts1=%s--mute_state=%d------\n", mts, mts1, atoi(mts1));	
						memcpy(m_text2send, f->data.ptr, f->datalen);
						
						m_f.data.ptr =  (void *)m_text2send;
						m_f.datalen = strlen(m_text2send) + 1;
						ast_log(LOG_DEBUG, "---response data=[%s]\n", m_text2send);	


						AST_LIST_TRAVERSE(&conf->userlist, m_usr, list) {							
							if(!m_usr)								
								break;
							if ((strcmp(m_usr->cid_num, mts)==0) &&(m_usr->user_status==1))
							{	
								m_usr->mute_status = atoi(mts1);								
								ast_write(m_usr->chan, &m_f) ;
								memset(m_text2send, 0x00, sizeof(m_text2send));
								sprintf(m_text2send, "ICS {\"no\":%d,\"fi\":\"%s\",\"na\":\"%s\",\"st\":%d,\"mt\":%d,\"hd\":%d}",  0, S_OR(m_usr->cid_num, "<unknown>"), 
											S_OR(m_usr->cid_name, "<unknown>"), m_usr->user_status, m_usr->mute_status, m_usr->hold_status);
								ast_log(LOG_DEBUG, "---response data=[%s]\n", m_text2send);	
								m_fag=1;
								break;
							}
						}
						if (m_fag==1)
						{
							m_f.data.ptr = ( void *)m_text2send;
							m_f.datalen = strlen(m_text2send) + 1;
							ast_log(LOG_DEBUG, "---response data=[%s]\n", m_text2send);					
							AST_LIST_TRAVERSE(&conf->userlist, m_usr, list) {							
								if(!m_usr)								
									break;
								ast_log(LOG_DEBUG, "====send to ===m_usr->cid_num=%s==\n", m_usr->cid_num);
								ast_write(m_usr->chan, &m_f) ;
							}
						}
						AST_LIST_UNLOCK(&confs);
					}
				}				
				
				if (f->frametype == AST_FRAME_NULL) {
					;
					/* Ignore NULL frames. It is perfectly normal to get these if the person is muted. */
				} else if (option_debug) {
					ast_log(LOG_DEBUG,
						"Got unrecognized frame on channel %s, f->frametype=%d,f->subclass=%d\n",
						ast_channel_name(chan), f->frametype, f->subclass.integer);
				}
				ast_frfree(f);
			} 
			else if (outfd > -1) 
			{
				//ast_log(LOG_DEBUG, "========chan->isstatectl=%d===\n", chan->isstatectl);
				if (ast_channel_isstatectl(chan)==0)
					res = read(outfd, buf, CONF_SIZE);
				else
					res = 0;
				if (res > 0) {
					usleep(1000);
				}
				else 
				{
					//ast_log(LOG_WARNING, "Failed to read frame: %s\n", strerror(errno));
					usleep(1000);
				}
			}
		}
		ast_log(LOG_DEBUG, "--------------Termination for loop------------\n");
	}

	AST_LIST_LOCK(&confs);
	ast_log(LOG_DEBUG, "---------------continue-------------\n");
	if (!ast_test_flag64(confflags, CONFFLAG_QUIET) && !ast_test_flag64(confflags, CONFFLAG_MONITOR) && !ast_test_flag64(confflags, CONFFLAG_ADMIN))
		conf_play(chan, conf, LEAVE);
	AST_LIST_UNLOCK(&confs);
	ast_log(LOG_DEBUG, "---------------continue-------------\n");

 outrun:
	AST_LIST_LOCK(&confs);
	ast_log(LOG_DEBUG, "---------------continue-------------\n");
	if (user->user_no) { /* Only cleanup users who really joined! */
		user->user_status=0;
		user->mute_status=0;
		user->hold_status=0;
		now = time(NULL);
		hr = (now - user->jointime) / 3600;
		min = ((now - user->jointime) % 3600) / 60;
		sec = (now - user->jointime) % 60;

		if (sent_event) {
			manager_event(EVENT_FLAG_CALL, "StatectlLeave",
				      "Channel: %s\r\n"
				      "Uniqueid: %s\r\n"
				      "Statectl: %s\r\n"
				      "Usernum: %d\r\n"
				      "CallerIDnum: %s\r\n"
				      "CallerIDname: %s\r\n"
				      "Duration: %ld\r\n",
				      ast_channel_name(chan), ast_channel_uniqueid(chan), conf->confno, 
				      user->user_no,
				      S_COR(ast_channel_caller(user->chan)->id.number.valid, ast_channel_caller(user->chan)->id.number.str, "<unknown>"), 
				      S_COR(ast_channel_caller(user->chan)->id.name.valid, ast_channel_caller(user->chan)->id.name.str, "<unknown>"),
				      (long)(now - user->jointime));
		}

		/***************statectl status send to ALL************************/
		AST_LIST_TRAVERSE(&conf->userlist, t_usr1, list) {
			if(!t_usr1)								
				break;
			ast_log(LOG_DEBUG, "=======t_usr1->cid_num=%s===t_usr1->user_status=%d==\n", t_usr1->cid_num, t_usr1->user_status);
			if ((strcmp(t_usr1->cid_num, S_COR(ast_channel_caller(chan)->id.number.valid, ast_channel_caller(chan)->id.number.str, "<unknown>"))==0) &&(t_usr1->user_status==1))
			{
				i_hangup = 1;
				ast_log(LOG_DEBUG, "------------i_hangup=%d---------\n", i_hangup);
			}
		}

		if (i_hangup==0)
		{
			memset(text2send1, 0x00, sizeof(text2send1));
			sprintf(text2send1, "ICS {\"no\":%d,\"fi\":\"%s\",\"na\":\"%s\",\"st\":%d,\"mt\":%d,\"hd\":%d}",  0, S_OR(user->cid_num, "<unknown>"), 
				S_OR(user->cid_name, "<unknown>"), 0, 0, 0);
			t_f1.data.ptr =(void*)text2send1;
			t_f1.datalen = strlen(text2send1) + 1;
			ast_log(LOG_DEBUG, "---------------continue-------------\n");
			AST_LIST_TRAVERSE(&conf->userlist, t_usr1, list) {							
				if(!t_usr1)								
					break;
				if ((t_usr1->user_status==1)&&(t_usr1->chan))
				{
					ast_log(LOG_DEBUG, "---status data=[%s]=sendto=name=[%s]---\n", text2send1, t_usr1->cid_num);
					ast_write(t_usr1->chan, &t_f1) ;
				}
			}
		}
		/***************statectl status send to ALL************************/
		ast_log(LOG_DEBUG, "---------------continue-------------\n");

		if (setusercount) {
			conf->users--;
			/* Update table */
			if (i_hangup==1)
			{
				conf->user_count--;
			}
			snprintf(members, sizeof(members), "%d", conf->users);
			ast_update_realtime("statectl", "confno", conf->confno, "members", members, NULL);
		}
		/* Remove ourselves from the list */
		if (i_hangup==1)
		{	
			AST_LIST_REMOVE(&conf->userlist, user, list);
		}
		ast_log(LOG_DEBUG, "---------------continue-------------\n");
		/* Change any states */
		if (!conf->users)
			ast_devstate_changed(AST_DEVICE_NOT_INUSE, (conf->isdynamic ? AST_DEVSTATE_NOT_CACHABLE : AST_DEVSTATE_CACHABLE), "statectl:%s", conf->confno);
		
		/* Return the number of seconds the user was in the conf */
		snprintf(statectlsecs, sizeof(statectlsecs), "%d", (int) (time(NULL) - user->jointime));
		pbx_builtin_setvar_helper(chan, "STATECTLSECS", statectlsecs);
	}
	if (i_hangup==1)
	{
		free(user);
	}
	ast_log(LOG_DEBUG, "---------------continue-------------\n");
	AST_LIST_UNLOCK(&confs);

	return ret;
}


static struct ast_conference *find_conf_realtime(struct ast_channel *chan, char *confno, int make, int dynamic,
						 char *dynamic_pin, size_t pin_buf_len, int refcount, struct ast_flags64 *confflags)
{
	struct ast_variable *var;
	struct ast_conference *cnf;

	/* Check first in the conference list */
	AST_LIST_LOCK(&confs);
	AST_LIST_TRAVERSE(&confs, cnf, list) {
		if (!strcmp(confno, cnf->confno)) 
			break;
	}
	if (cnf) {
		cnf->refcount += refcount;
	}
	AST_LIST_UNLOCK(&confs);

	if (!cnf) {
		char *pin = NULL, *pinadmin = NULL; /* For temp use */
		char *maxcount = NULL;//anita-statectl
		
		var = ast_load_realtime("statectl", "confno", confno, NULL);

		if (!var)
			return NULL;

		while (var) {
			if (!strcasecmp(var->name, "pin")) {
				pin = ast_strdupa(var->value);
			} else if (!strcasecmp(var->name, "adminpin")) {
				pinadmin = ast_strdupa(var->value);
			}
			var = var->next;
		}
		ast_variables_destroy(var);
		
		cnf = build_conf(confno, pin ? pin : "", pinadmin ? pinadmin : "", maxcount ? maxcount : "", make, dynamic, refcount);//anita-statectl
	}

	if (cnf) {
		if (confflags->flags && !cnf->chan &&
		    !ast_test_flag64(confflags, CONFFLAG_QUIET) &&
		    ast_test_flag64(confflags, CONFFLAG_INTROUSER)) {
			ast_log(LOG_WARNING, "No Zap channel available for conference, user introduction disabled (is chan_zap loaded?)\n");
			ast_clear_flag64(confflags, CONFFLAG_INTROUSER);
		}
		
		if (confflags->flags && !cnf->chan &&
		    ast_test_flag64(confflags, CONFFLAG_RECORDCONF)) {
			ast_log(LOG_WARNING, "No Zap channel available for conference, conference recording disabled (is chan_zap loaded?)\n");
			ast_clear_flag64(confflags, CONFFLAG_RECORDCONF);
		}
	}

	return cnf;
}


static struct ast_conference *find_conf(struct ast_channel *chan, char *confno, int make, int dynamic,
					char *dynamic_pin, size_t pin_buf_len, int refcount, struct ast_flags64 *confflags)
{
	struct ast_config *cfg;
	struct ast_variable *var;
	struct ast_conference *cnf;
	struct ast_flags config_flags = { 0 };
	char *parse;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(confno);
		AST_APP_ARG(pin);
		AST_APP_ARG(pinadmin);
		AST_APP_ARG(maxcount);//anita-statectl
	);

	/* Check first in the conference list */
	AST_LIST_LOCK(&confs);
	AST_LIST_TRAVERSE(&confs, cnf, list) {
		if (!strcmp(confno, cnf->confno)) 
			break;
	}
	if (cnf){
		cnf->refcount += refcount;
	}
	AST_LIST_UNLOCK(&confs);

	if (!cnf) {
		if (dynamic) {
			/* No need to parse statectl.conf */
			ast_log(LOG_DEBUG, "Building dynamic conference '%s'\n", confno);
			if (dynamic_pin) {
				if (dynamic_pin[0] == 'q') {
					/* Query the user to enter a PIN */
					if (ast_app_getdata(chan, "conf-getpin", dynamic_pin, pin_buf_len - 1, 0) < 0)
						return NULL;
				}
				cnf = build_conf(confno, dynamic_pin, "", "", make, dynamic, refcount);//anita-statectl
			} else {
				cnf = build_conf(confno, "", "", "", make, dynamic, refcount);//anita-statectl
			}
		} else {
			/* Check the config */
			cfg = ast_config_load(CONFIG_FILE_NAME, config_flags);
			if (!cfg) {
				ast_log(LOG_WARNING, "No %s file :(\n", CONFIG_FILE_NAME);
				return NULL;
			}
			for (var = ast_variable_browse(cfg, "rooms"); var; var = var->next) {
				if (strcasecmp(var->name, "conf"))
					continue;
				
				if (!(parse = ast_strdupa(var->value)))
					return NULL;
				
				AST_NONSTANDARD_APP_ARGS(args, parse, ',');
				if (!strcasecmp(args.confno, confno)) {
					/* Bingo it's a valid conference */
					cnf = build_conf(args.confno,
							S_OR(args.pin, ""),
							S_OR(args.pinadmin, ""),
							S_OR(args.maxcount, ""),//anita-statectl
							make, dynamic, refcount);
					break;
				}
			}
			if (!var) {
				ast_log(LOG_DEBUG, "%s isn't a valid conference\n", confno);
			}
			ast_config_destroy(cfg);
		}
	} else if (dynamic_pin) {
		/* Correct for the user selecting 'D' instead of 'd' to have
		   someone join into a conference that has already been created
		   with a pin. */
		if (dynamic_pin[0] == 'q')
			dynamic_pin[0] = '\0';
	}

	if (cnf) {
		if (confflags->flags && !cnf->chan &&
		    !ast_test_flag64(confflags, CONFFLAG_QUIET) &&
		    ast_test_flag64(confflags, CONFFLAG_INTROUSER)) {
			ast_log(LOG_WARNING, "No Zap channel available for conference, user introduction disabled (is chan_zap loaded?)\n");
			ast_clear_flag64(confflags, CONFFLAG_INTROUSER);
		}
		
		if (confflags->flags && !cnf->chan &&
		    ast_test_flag64(confflags, CONFFLAG_RECORDCONF)) {
			ast_log(LOG_WARNING, "No Zap channel available for conference, conference recording disabled (is chan_zap loaded?)\n");
			ast_clear_flag64(confflags, CONFFLAG_RECORDCONF);
		}
	}

	return cnf;
}
#define ROOM_MAX_COUNT_CODE 	1
#define ROOM_PWD_ERROR_CODE 	2
#define ROOM_ACL_ALLOW_CODE 	3

/*! \brief The statectl() application */
static int conf_exec(struct ast_channel *chan, const char *data)
{
	int res=-1;
	struct ast_module_user *u;
	char confno[MAX_CONFNUM] = "";
	int allowretry = 0;
	int retrycnt = 0;
	struct ast_conference *cnf = NULL;
	struct ast_flags64 confflags = {0};
	int dynamic = 0;
	int empty = 0, empty_no_pin = 0;
	int always_prompt = 0;
	const char *notdata;
	char *info, the_pin[MAX_PIN] = "";
	struct ast_flags config_flags = { 0 };
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(confno);
		AST_APP_ARG(options);
		AST_APP_ARG(pin);
	);
	char *optargs[OPT_ARG_ARRAY_SIZE] = { NULL, };

	u = ast_module_user_add(chan);

	if (ast_strlen_zero(data)) {
		allowretry = 1;
		notdata = "";
	} else {
		notdata = data;
	}
	
	info = ast_strdupa(notdata);

	AST_STANDARD_APP_ARGS(args, info);	

	if (args.confno) {
		ast_copy_string(confno, args.confno, sizeof(confno));
		if (ast_strlen_zero(confno)) {
			allowretry = 1;
		}
	}
	
	if (args.pin)
		ast_copy_string(the_pin, args.pin, sizeof(the_pin));

	if (args.options) {
		ast_app_parse_options64(statectl_opts, &confflags, optargs, args.options);
		dynamic = ast_test_flag64(&confflags, CONFFLAG_DYNAMIC | CONFFLAG_DYNAMICPIN);
		if (ast_test_flag64(&confflags, CONFFLAG_DYNAMICPIN) && !args.pin)
			strcpy(the_pin, "q");

		empty = ast_test_flag64(&confflags, CONFFLAG_EMPTY | CONFFLAG_EMPTYNOPIN);
		empty_no_pin = ast_test_flag64(&confflags, CONFFLAG_EMPTYNOPIN);
		always_prompt = ast_test_flag64(&confflags, CONFFLAG_ALWAYSPROMPT);
	}

	do {
		if (retrycnt > 3)
			allowretry = 0;
		if (empty) {
			int i;
			struct ast_config *cfg;
			struct ast_variable *var;
			int confno_int;

			/* We only need to load the config file for static and empty_no_pin (otherwise we don't care) */
			if ((empty_no_pin) || (!dynamic)) {
				cfg = ast_config_load(CONFIG_FILE_NAME, config_flags);
				if (cfg) {
					var = ast_variable_browse(cfg, "rooms");
					while (var) {
						if (!strcasecmp(var->name, "conf")) {
							char *stringp = ast_strdupa(var->value);
							if (stringp) {
								char *confno_tmp = strsep(&stringp, "|,");
								int found = 0;
								if (!dynamic) {
									/* For static:  run through the list and see if this conference is empty */
									AST_LIST_LOCK(&confs);
									AST_LIST_TRAVERSE(&confs, cnf, list) {
										if (!strcmp(confno_tmp, cnf->confno)) {
											/* The conference exists, therefore it's not empty */
											found = 1;
											break;
										}
									}
									AST_LIST_UNLOCK(&confs);
									cnf = NULL;
									if (!found) {
										/* At this point, we have a confno_tmp (static conference) that is empty */
										if ((empty_no_pin && ast_strlen_zero(stringp)) || (!empty_no_pin)) {
											/* Case 1:  empty_no_pin and pin is nonexistent (NULL)
											 * Case 2:  empty_no_pin and pin is blank (but not NULL)
											 * Case 3:  not empty_no_pin
											 */
											ast_copy_string(confno, confno_tmp, sizeof(confno));
											break;
											/* XXX the map is not complete (but we do have a confno) */
										}
									}
								}
							}
						}
						var = var->next;
					}
					ast_config_destroy(cfg);
				}
			}

			/* Select first conference number not in use */
			if (ast_strlen_zero(confno) && dynamic) {
				AST_LIST_LOCK(&confs);
				for (i = 0; i < sizeof(conf_map) / sizeof(conf_map[0]); i++) {
					if (!conf_map[i]) {
						snprintf(confno, sizeof(confno), "%d", i);
						conf_map[i] = 1;
						break;
					}
				}
				AST_LIST_UNLOCK(&confs);
			}

			/* Not found? */
			if (ast_strlen_zero(confno)) {
				res = ast_streamfile(chan, "conf-noempty", ast_channel_language(chan));
				if (!res)
					ast_waitstream(chan, "");
			} else {
				if (sscanf(confno, "%d", &confno_int) == 1) {
					res = ast_streamfile(chan, "conf-enteringno", ast_channel_language(chan));
					if (!res) {
						ast_waitstream(chan, "");
						res = ast_say_digits(chan, confno_int, "", ast_channel_language(chan));
					}
				} else {
					ast_log(LOG_ERROR, "Could not scan confno '%s'\n", confno);
				}
			}
		}

		while (allowretry && (ast_strlen_zero(confno)) && (++retrycnt < 4)) {
			/* Prompt user for conference number */
			res = ast_app_getdata(chan, "conf-getconfno", confno, sizeof(confno) - 1, 0);
			if (res < 0) {
				/* Don't try to validate when we catch an error */
				confno[0] = '\0';
				allowretry = 0;
				break;
			}
		}
		if (!ast_strlen_zero(confno)) {
			/* Check the validity of the conference */
			cnf = find_conf(chan, confno, 1, dynamic, the_pin,sizeof(the_pin), 1, &confflags);
			if (!cnf) {
				cnf = find_conf_realtime(chan, confno, 1, dynamic, the_pin, sizeof(the_pin), 1, &confflags);
			}

			if (!cnf) {
				res = ast_streamfile(chan, "conf-invalid", ast_channel_language(chan));
				if (!res)
					ast_waitstream(chan, "");
				res = -1;
				if (allowretry)
					confno[0] = '\0';
			} else {
				//anita,process business mode
				if(cnf->business_mode == BUSI_PTT){
					ast_channel_business_mode_set(chan,BUSI_PTT);
					memcpy(ast_channel_pttip(chan), &(cnf->pttip), sizeof(struct sockaddr_in));
					memcpy(ast_channel_pttid(chan), cnf->pttid, sizeof(ast_channel_pttid(chan)));
				}else if(cnf->business_mode == BUSI_AUDIO_CNF || cnf->business_mode == BUSI_VEDIO_CNF){
					ast_channel_business_mode_set(chan,cnf->business_mode);
					memcpy(ast_channel_relayip(chan), &(cnf->MixingServerIP), sizeof(struct sockaddr_in));
					if ((ntohs(ast_channel_relayip(chan)->sin_port))==0)
						ast_channel_isstatectl_set(chan,0);
					else
						ast_channel_isstatectl_set(chan,1);
				}
				//process autherize
				int result_num = 0;
				int maxusers = 0;
				if(cnf->auth_mode == AUTH_WHITELIST){
					ast_log(LOG_DEBUG,"44444444444group %s auth mode is whitelist\n", cnf->confno);
//					result_num = realtime_groups_whitelist(chan->cid.cid_num, cnf->confno);
					result_num = realtime_groups_whitelist(ast_channel_caller(chan)->id.number.str, cnf->confno);
					ast_log(LOG_DEBUG,"table bandinfo has %d result\n", result_num);
					if(result_num < 0){
						ast_log(LOG_ERROR,"user %s whitelist auth failed\n", ast_channel_caller(chan)->id.number.str);
						//res = -1;
						res = ROOM_ACL_ALLOW_CODE;
						allowretry = 0;
						break;
					}
				}

				if((cnf->auth_mode == AUTH_MAXCOUNT) && (!ast_strlen_zero(cnf->maxcount))){
					ast_log(LOG_DEBUG,"555555555555group %s auth mode is maxcount\n", cnf->confno);
					ast_log(LOG_DEBUG,"-----conference %s cnf->users = %d, maxcount = %d----\n",cnf->confno, cnf->users,atoi(cnf->maxcount));
					maxusers = atoi(cnf->maxcount);
					if(cnf->users >= maxusers){
						ast_log(LOG_DEBUG,"-----conference %s has reached maxcount %d----\n",cnf->confno, maxusers);
						//res = -1;
						res = ROOM_MAX_COUNT_CODE;
						allowretry = 0;
						break;
					}
				}

				if ((cnf->auth_mode == AUTH_PWD)&&//anita
					((!ast_strlen_zero(cnf->pin) &&
				     !ast_test_flag64(&confflags, CONFFLAG_ADMIN)) ||
				    (!ast_strlen_zero(cnf->pinadmin) &&
				     ast_test_flag64(&confflags, CONFFLAG_ADMIN)))) {
					char pin[MAX_PIN] = "";
					int j;

					/* Allow the pin to be retried up to 3 times */
					for (j = 0; j < 3; j++) {
						if (*the_pin && (always_prompt == 0)) {
							ast_copy_string(pin, the_pin, sizeof(pin));
							res = 0;
						} else {
							/* Prompt user for pin if pin is required */
							res = ast_app_getdata(chan, "conf-getpin", pin + strlen(pin), sizeof(pin) - 1 - strlen(pin), 0);
						}
						if (res >= 0) {
							if (!strcasecmp(pin, cnf->pin) ||
							    (!ast_strlen_zero(cnf->pinadmin) &&
							     !strcasecmp(pin, cnf->pinadmin))) {
								/* Pin correct */
								allowretry = 0;
								if (!ast_strlen_zero(cnf->pinadmin) && !strcasecmp(pin, cnf->pinadmin)) 
									ast_set_flag64(&confflags, CONFFLAG_ADMIN);
								/* Run the conference */
								res = conf_run(chan, cnf, &confflags, optargs);
								break;
							} else {
								/* Pin invalid */
								if (!ast_streamfile(chan, "conf-invalidpin",ast_channel_language(chan))) {
									res = ast_waitstream(chan, AST_DIGIT_ANY);
									ast_stopstream(chan);
								}
								else {
									ast_log(LOG_WARNING, "Couldn't play invalid pin msg!\n");
									break;
								}
								if (res < 0)
									break;
								pin[0] = res;
								pin[1] = '\0';
								//res = -1;
								res = ROOM_PWD_ERROR_CODE;
								if (allowretry)
									confno[0] = '\0';
							}
						} else {
							/* failed when getting the pin */
							//res = -1;
							res = ROOM_PWD_ERROR_CODE;
							allowretry = 0;
							/* see if we need to get rid of the conference */
							break;
						}

						/* Don't retry pin with a static pin */
						if (*the_pin && (always_prompt==0)) {
							break;
						}
					}
				} else {
					allowretry = 0;
					ast_log(LOG_DEBUG,"-----6666666666conference %s cnf->users = %d, cnf->fd = %d----\n",cnf->confno, cnf->users,cnf->fd);

					res = conf_run(chan, cnf, &confflags, optargs);
				}
				dispose_conf(cnf);
				cnf = NULL;
			}
		}
	} while (allowretry);
	if (cnf)
		dispose_conf(cnf);

	ast_module_user_remove(u);
	
	return res;
}


#define ZT_MAX_NUM_BUFS		 32
#define ZT_DEFAULT_NUM_BUFS	 2

static void load_config_statectl(void)
{
	struct ast_config *cfg;
	struct ast_flags config_flags = { 0 };
	const char *val;

	if (!(cfg = ast_config_load(CONFIG_FILE_NAME, config_flags)))
		return;

	val = ast_variable_retrieve(cfg, "general", "query_interval");
	if (val) {
		query_interval=atoi(val);
	}
	ast_log(LOG_DEBUG, "query_interval=%d\n", query_interval);
	val = ast_variable_retrieve(cfg, "general", "Mixing_media_port");
	if (val) {
		Mixing_media_port=atoi(val);
	}
	ast_log(LOG_DEBUG, "Mixing_media_port=%d\n", Mixing_media_port);

	val = ast_variable_retrieve(cfg, "general", "Mixing_media_ip");
	if (!ast_strlen_zero(val))
		ast_copy_string(Mixing_media_ip, val, sizeof(Mixing_media_ip));
	ast_log(LOG_DEBUG, "Mixing_media_ip=%s\n", Mixing_media_ip);
	
	val = ast_variable_retrieve(cfg, "general", "prompt_sounds_path");
	if (!ast_strlen_zero(val))
		ast_copy_string(prompt_sounds_path, val, sizeof(prompt_sounds_path));
	
	ast_config_destroy(cfg);
}

static int load_config(int reload)
{
	int res = 0;
	load_config_statectl();
	return res;
}

static int unload_module(void)
{
	int res = 0;
	
	ast_cli_unregister_multiple(cli_statectl, ARRAY_LEN(cli_statectl));
	res |= ast_unregister_application(app);
	ast_module_user_hangup_all();
	ast_unload_realtime("statectl");
	return res;
}

static int load_module(void)
{
	int res = 0;

	res |= load_config(0);
	ast_cli_register_multiple(cli_statectl, ARRAY_LEN(cli_statectl));
	res |= ast_register_application_xml(app, conf_exec);
	ast_realtime_require_field("statectl", "confno", RQ_UINTEGER2, 3, "members", RQ_UINTEGER1, 3, NULL);
	return res;
}

static int reload(void)
{
	ast_unload_realtime("statectl");
	return load_config(1);
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_LOAD_ORDER, "StateCtl conference bridge",
		.load = load_module,
		.unload = unload_module,
		.reload = reload,
		.load_pri = AST_MODPRI_DEVSTATE_PROVIDER,
	       );

