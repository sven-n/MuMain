CREATE TABLE [dbo].[CustomAttack](
	[Name] [varchar](10) NOT NULL,
	[Active] [smallint] NOT NULL,
	[Skill] [smallint] NULL,
	[Map] [smallint] NULL,
	[PosX] [smallint] NULL,
	[PosY] [smallint] NULL,
	[AutoBuff] [smallint] NULL,
	[OffPvP] [smallint] NULL,
	[AutoReset] [smallint] NULL,
	[AutoAddStr] [int] NULL,
	[AutoAddAgi] [int] NULL,
	[AutoAddVit] [int] NULL,
	[AutoAddEne] [int] NULL,
	[AutoAddCmd] [int] NULL,
 CONSTRAINT [PK_CustomAttack] PRIMARY KEY CLUSTERED 
(
	[Name] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]