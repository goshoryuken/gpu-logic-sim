module circuit(clk, q);
  input clk;
  output q;
  wire d;

  $_DFF_P_ ff1 (.C(clk), .D(d), .Q(q));
  not inv1(d, q);

  
endmodule