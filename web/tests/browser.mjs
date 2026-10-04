import {readFileSync} from 'node:fs';
import ts from 'typescript';
import {createRequire} from 'node:module';
import assert from 'node:assert/strict';
const require=createRequire(import.meta.url);
const {chromium}=require(process.env.PLAYWRIGHT_MODULE || 'playwright-core');
const reportCode = ts.transpileModule(readFileSync(new URL('../src/report.ts', import.meta.url), 'utf8'), {compilerOptions: {target:ts.ScriptTarget.ES2022,module:ts.ModuleKind.ESNext}}).outputText;
const browser=await chromium.launch({executablePath:process.env.CHROMIUM_PATH || '/usr/bin/chromium',headless:true,args:['--no-sandbox']});
try {
 for(const width of [320,390,1440]){
  const page=await browser.newPage({viewport:{width,height:1000}});
  const errors=[];page.on('pageerror',e=>errors.push(e.message));
  const response=await page.goto(process.env.BASE_URL || 'http://127.0.0.1:4175');assert.equal(response.status(),200);
  await page.getByRole('heading',{name:'What this machine measured'}).waitFor();
  assert.equal(await page.locator('tbody tr').count(),3);
  assert.equal(await page.locator('svg text').evaluateAll(nodes => nodes.every(node => parseFloat(getComputedStyle(node).fontSize) * node.getScreenCTM().a >= 12)), true, 'Diagram labels must remain readable on mobile');
  assert.match(await page.locator('meta[name="description"]').getAttribute('content'), /serial/i);
  await page.getByRole('link',{name:'Strategy',exact:true}).click();assert.match(page.url(),/#strategy$/);
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),true);
  assert.deepEqual(errors,[]);
  const table = page.getByRole('region', {name:'Recorded runtime table, scroll horizontally if needed'});
  await table.focus();
  assert.equal(await table.evaluate(node => document.activeElement === node), true);
  await page.setViewportSize({width: width === 1440 ? 320 : 1440, height:1000});
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth), true, 'Resize must not overflow the page');
  await page.setViewportSize({width,height:1000});
  await page.screenshot({path:`/tmp/c10-final-${width}.png`,fullPage:true});
  for (const [value, role, heading] of [[{results:[]}, 'status', 'No measurements available'], [null, 'alert', 'Results could not be displayed']]) {
    await page.evaluate(async ({code, value}) => {
      const report = await import(URL.createObjectURL(new Blob([code], {type:'text/javascript'})));
      document.querySelector('#app').innerHTML = report.renderReport(value);
    }, {code:reportCode, value});
    await page.getByRole(role).getByRole('heading', {name:heading}).waitFor();
    assert.equal(await page.locator('.bar').count(), 0);
    assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth), true);
  }
  assert.deepEqual(errors, []);
  await page.close();
 }
 console.log('PASS: served Chromium, mobile/desktop, runtime rows, keyboard table access, resize, empty/error states, navigation, no overflow or page errors');
} finally {await browser.close()}
