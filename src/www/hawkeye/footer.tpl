<!-- END HAWKYE CONTENT -->

<script tal:condition="opsuccess">window.setTimeout("new Effect.Highlight('opsuccess');", 1500); window.setTimeout("Effect.Fade('opsuccess');", 5000);</script>
<script tal:condition="info">window.setTimeout("new Effect.Highlight('msg');", 1500); window.setTimeout("Effect.Fade('msg');", 5000);</script>
<script tal:condition="error">window.setTimeout("Effect.Pulsate('err');", 1000);</script>
</div>
</div>
</body>
</html>