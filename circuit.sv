module circuit(a, b, c, out);
  wire _0_;
  input a;
  input b;
  input c;
  output out;

  and _2_(_0_, a, b);
  or _3_(out, _0_, c);
endmodule