import {createRequire} from 'node:module';
import assert from 'node:assert/strict';
const require=createRequire(import.meta.url);
const {chromium}=require(process.env.PLAYWRIGHT_MODULE || 'playwright-core');
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
  await page.screenshot({path:`/tmp/c10-final-${width}.png`,fullPage:true});
  await page.close();
 }
 console.log('PASS: served Chromium, mobile/desktop, runtime rows, navigation, no overflow or page errors');
} finally {await browser.close()}
