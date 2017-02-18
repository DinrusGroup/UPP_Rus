topic "Incremental Development of Database Models/Schemas";[2 $$0,0#00000000000000000000000000000000:Default]
[l288;i1120;a17;O9;~~~.1408;2 $$1,0#10431211400427159095818037425705:param]
[a83;*R6 $$2,5#31310162474203024125188417583966:caption]
[b83;*4 $$3,5#07864147445237544204411237157677:title]
[i288;O9;C2 $$4,6#40027414424643823182269349404212:item]
[l25;b25;a25;2 $$5,5#45413000475342174754091244180557:text]
[l288;b17;a17;2 $$6,6#27521748481378242620020725143825:desc]
[l321;t246;C@5;1 $$7,7#20902679421464641399138805415013:code]
[b2503;2 $$8,0#65142375456100023862071332075487:separator]
[*@(0.0.255)2 $$9,0#83433469410354161042741608181528:base]
[t4167;C2 $$10,0#37138531426314131251341829483380:class]
[l288;a17;*1 $$11,11#70004532496200323422659154056402:requirement]
[i417;b42;a42;O9;~~~.416;2 $$12,12#10566046415157235020018451313112:tparam]
[b167;C2 $$13,13#92430459443460461911108080531343:item1]
[i288;a42;O9;C2 $$14,14#77422149456609303542238260500223:item2]
[*@2$(0.128.128)2 $$15,15#34511555403152284025741354420178:NewsDate]
[l321;*C$7;2 $$16,16#03451589433145915344929335295360:result]
[l321;b83;a83;*C$7;2 $$17,17#07531550463529505371228428965313:result`-line]
[l160;t4167;*C+117 $$18,5#88603949442205825958800053222425:package`-title]
[2 $$19,0#53580023442335529039900623488521:gap]
[t4167;C2 $$20,20#70211524482531209251820423858195:class`-nested]
[b50;2 $$21,21#03324558446220344731010354752573:Par]
[b83;_R+117 $$22,5#50327538467692101372348457405326:heading1]
[b50;a25;*2 $$23,24#10189570498413014020290115411120:heading2]
[l25;b25;a25;R2 $$24,24#24747716437513346533020414014942:text2]
[l25;b25;a25;*/2 $$25,25#14962814463678751281510843401240:code`_header]
[{_}%EN-US 
[s3; U`+`+ SQL&]
[s22; Incremental Development of Database Models/Schemas&]
[s25; Creating the database:&]
[s7; SqlSchema sch(PGSQL);&]
[s7; All`_Tables(sch);&]
[s24; The [C All`_Tables] function will generate SQL scripts from your 
schema (.sch file) definition and store them internally. This 
function is defined by including the .sch file. It is a sort 
of .sch interface point. All`_Tables does not execute scripts, 
it creates them as Strings inside SqlSchema.&]
[s25; Checking for changes and executing SQL creation/update/drop 
scripts:&]
[s7; StdStatementExecutor se(m`_session);&]
[s7; Progress p;&]
[s7; p.SetText(t`_(`"Creating database`"));&]
[s7; if(sch.ScriptChanged(SqlSchema`::UPGRADE))&]
[s7; -|PostgreSQLPerformScript(sch.Upgrade(),se, p);&]
[s7; if(sch.ScriptChanged(SqlSchema`::ATTRIBUTES)) `{&]
[s7; -|PostgreSQLPerformScript(sch.Attributes(),se, p);&]
[s7; `}&]
[s7; if(sch.ScriptChanged(SqlSchema`::CONFIG)) `{&]
[s7; -|PostgreSQLPerformScript(sch.ConfigDrop(),se, p);&]
[s7; -|PostgreSQLPerformScript(sch.Config(),se, p);&]
[s7; `}&]
[s24; The [C ScriptChanged()] function compares the current existing 
SQL script stored by [C SaveNormal] in previous run of code (by 
default in the .exe dir in Win32, and `~/.upp/appname in Linux) 
to the one generated by the application at runtime. If they are 
different, then [C ScriptChanged()] will return true. The purpose 
 is to avoid running scripts each time (it can be time consuming). 
&]
[s24; [C ScriptChanged()] also takes as arguments the directory to look 
in for the scripts and the base`-name of the scripts. Customizing 
the location of the script files is especially useful for connecting 
to multiple databases.&]
[s24; The UPGRADE script contains the SQL code to create all tables 
and columns, however it does so by creating tables only with 
their first column, then adding all columns using `'ALTER TABLE 
... ADD COLUMN`' one by one. This allows incemental development 
of the model. At the start of the application, the model gets 
upgraded, and commands to create all columns and tables that 
already exist simply fail, but any new columns or tables will 
be added. Note that this model does not support removing columns 
or changing datatype; that has to be done manually.&]
[s24; ATTRIBUTES script adds any `"attributes`", namely constraints 
or indicies. The main reason to have this separated is because 
sometimes contraints have to be added only after the tables are 
defined (Ie. forward foreign keys). A secondary reason is that 
U`+`+ also generates `"drop`" scripts; sometimes it is useful, 
when maintaining app, to drop all or some contraints and indicies 
and recreate them later.&]
[s24; CONFIG contains any data configuration; usually inserts of `"metadata`".&]
[s24; Note: in the above code, [C Progress p] is used as a callback function 
that updates the progress indicator (optional);&]
[s25; Saving your SQL script files to disk:&]
[s7; sch.SaveNormal();&]
[s24; [C SaveNormal] saves all scripts into a configuration directory 
(.exe dir in Win32, `~/.upp/appname in Linux). This function 
also takes as arguments the directory and base file name of the 
script files.&]
[s24;C1 SaveNormal(const char `*dir `= NULL¸ const char `*name `= 
NULL)&]
[s24; Note: In some cases, script execution can be blocked in your 
program if you do not customize the directory location and/or 
the base`-name of the script files. For example, say you are 
using 2 identical database schemas and you save to one database, 
saving the script files using [C SaveNormal()]. Then, you attempt 
to run it against another database, the script files on disk 
are already created and can be the same, and they will not be 
recognized as changed by your program. If you only have 1 database 
and schema, this should not be an issue.&]
[s24; &]
[s24; &]
[s24; &]
[s0; ]