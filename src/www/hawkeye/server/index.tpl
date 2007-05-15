%(include|header.tpl)s

<p>This page enables you to enable and disable specific agents during runtime for change at next restart.</p>
<p>Please note that you <strong>must restart <c>bongo-manager</c></strong> for these changes to take effect, since they're only to do with the agent list at startup, not dynamic starting and stopping.</p>

<p>

<div tal:condition="success" style="text-align: center;">
<form method="post">
	<input type="hidden" name="command" value="agentstatus" />
	<table class="htable" cellspacing="0">
	<tr class="highlight hrow">
		<th style="padding: 10px;">Enabled</th>
		<th style="padding: 10px;">Agent Name</th>
	</tr>
	<tr tal:repeat="agent agentlist" tal:attributes="class string:highlight${repeat/agent/odd}">
		<td style="height: 24px">
		<input type="checkbox" tal:attributes="checked agent/enabled;name agent/name" />
		</td>
		<td tal:content="agent/name">unknown agent</td>
	</tr>
	</table>
	<br />
	<span class="button"><button type="submit" value="Save">Save</button></span>
</form>
</div>

</p>

	<br />
%(include|footer.tpl)s